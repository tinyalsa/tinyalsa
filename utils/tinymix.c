/* tinymix.c
**
** Copyright 2011, The Android Open Source Project
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of The Android Open Source Project nor the names of
**       its contributors may be used to endorse or promote products derived
**       from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY The Android Open Source Project ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL The Android Open Source Project BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
** DAMAGE.
*/

#include <tinyalsa/asoundlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#define OPTPARSE_IMPLEMENTATION
#include "optparse.h"

static void list_controls(struct mixer *mixer, int print_all);

static void print_control_values(struct mixer_ctl *control);
static void print_control_values_by_name_or_id(struct mixer *mixer, const char *name_or_id);

static int set_values(struct mixer *mixer, const char *control,
                             char **values, unsigned int num_values);

static void print_enum(struct mixer_ctl *ctl);

void usage(void)
{
    printf("usage: tinymix [options] <command>\n");
    printf("options:\n");
    printf("\t-h, --help               : prints this help message and exits\n");
    printf("\t-v, --version            : prints this version of tinymix and exits\n");
    printf("\t-D, --card NUMBER        : specifies the card number of the mixer\n");
    printf("\n");
    printf("commands:\n");
    printf("\tget NAME|ID              : prints the values of a control\n");
    printf("\tset NAME|ID VALUE(S) ... : sets the value of a control\n");
    printf("\t\tVALUE(S): integers, percents, and relative values\n");
    printf("\t\t\tIntegers: 0, 100, -100 ...\n");
    printf("\t\t\tPercents: 0%%, 100%% ...\n");
    printf("\t\t\tRelative values: 1+, 1-, 1%%+, 2%%+ ...\n");
    printf("\tcontrols                 : lists controls of the mixer\n");
    printf("\tcontents                 : lists controls of the mixer and their contents\n");
}

void version(void)
{
    printf("tinymix version 2.0 (tinyalsa version %s)\n", TINYALSA_VERSION_STRING);
}

static int is_command(char *arg) {
    return strcmp(arg, "get") == 0 || strcmp(arg, "set") == 0 ||
            strcmp(arg, "controls") == 0 || strcmp(arg, "contents") == 0;
}

static int find_command_position(int argc, char **argv)
{
    for (int i = 0; i < argc; ++i) {
        if (is_command(argv[i])) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char **argv)
{
    int card = 0;
    struct optparse opts;
    static struct optparse_long long_options[] = {
        { "card",    'D', OPTPARSE_REQUIRED },
        { "version", 'v', OPTPARSE_NONE     },
        { "help",    'h', OPTPARSE_NONE     },
        { 0, 0, 0 }
    };

    // optparse_long may change the order of the argv list. Duplicate one for parsing the options.
    char **argv_options_list = (char **) calloc(argc + 1, sizeof(char *));
    if (!argv_options_list) {
        fprintf(stderr, "Failed to allocate options list\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < argc; ++i) {
        argv_options_list[i] = argv[i];
    }

    optparse_init(&opts, argv_options_list);
    /* Detect the end of the options. */
    int c;
    while ((c = optparse_long(&opts, long_options, NULL)) != -1) {
        switch (c) {
        case 'D':
            card = atoi(opts.optarg);
            break;
        case 'h':
            usage();
            free(argv_options_list);
            return EXIT_SUCCESS;
        case 'v':
            version();
            free(argv_options_list);
            return EXIT_SUCCESS;
        case '?':
        default:
            break;
        }
    }
    free(argv_options_list);

    struct mixer *mixer = mixer_open(card);
    if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return EXIT_FAILURE;
    }

    int command_position = find_command_position(argc, argv);
    if (command_position < 0) {
        usage();
        mixer_close(mixer);
        return EXIT_FAILURE;
    }

    char *cmd = argv[command_position];
    if (strcmp(cmd, "get") == 0) {
        if (command_position + 1 >= argc) {
            fprintf(stderr, "no control specified\n");
            mixer_close(mixer);
            return EXIT_FAILURE;
        }
        print_control_values_by_name_or_id(mixer, argv[command_position + 1]);
        printf("\n");
    } else if (strcmp(cmd, "set") == 0) {
        if (command_position + 1 >= argc) {
            fprintf(stderr, "no control specified\n");
            mixer_close(mixer);
            return EXIT_FAILURE;
        }
        if (command_position + 2 >= argc) {
            fprintf(stderr, "no value(s) specified\n");
            mixer_close(mixer);
            return EXIT_FAILURE;
        }
        int res = set_values(mixer,
                             argv[command_position + 1],
                             &argv[command_position + 2],
                             argc - command_position - 2);
        if (res != 0) {
            mixer_close(mixer);
            return EXIT_FAILURE;
        }
    } else if (strcmp(cmd, "controls") == 0) {
        list_controls(mixer, 0);
    } else if (strcmp(cmd, "contents") == 0) {
        list_controls(mixer, 1);
    } else {
        fprintf(stderr, "unknown command '%s'\n", cmd);
        usage();
        mixer_close(mixer);
        return EXIT_FAILURE;
    }

    mixer_close(mixer);
    return EXIT_SUCCESS;
}

static int isnumber(const char *str) {
    char *end;

    if (str == NULL || strlen(str) == 0)
        return 0;

    strtol(str, &end, 0);
    return strlen(end) == 0;
}

static void list_controls(struct mixer *mixer, int print_all)
{
    struct mixer_ctl *ctl;
    const char *name, *type;
    unsigned int num_ctls, num_values, device;
    unsigned int i;

    num_ctls = mixer_get_num_ctls(mixer);

    printf("Number of controls: %u\n", num_ctls);

    if (print_all)
        printf("ctl\ttype\tnum\t%-40s\tdevice\tvalue\n", "name");
    else
        printf("ctl\ttype\tnum\t%-40s\tdevice\n", "name");

    for (i = 0; i < num_ctls; i++) {
        ctl = mixer_get_ctl(mixer, i);

        name = mixer_ctl_get_name(ctl);
        type = mixer_ctl_get_type_string(ctl);
        num_values = mixer_ctl_get_num_values(ctl);
        device = mixer_ctl_get_device(ctl);
        printf("%u\t%s\t%u\t%-40s\t%u\t", i, type, num_values, name, device);
        if (print_all)
            print_control_values(ctl);
        printf("\n");
    }
}

static void print_enum(struct mixer_ctl *ctl)
{
    unsigned int num_enums;
    unsigned int i;
    unsigned int value;
    const char *string;

    num_enums = mixer_ctl_get_num_enums(ctl);
    value = mixer_ctl_get_value(ctl, 0);

    for (i = 0; i < num_enums; i++) {
        string = mixer_ctl_get_enum_string(ctl, i);
        printf("%s%s, ", value == i ? "> " : "", string);
    }
}

static void print_control_values_by_name_or_id(struct mixer *mixer, const char *name_or_id)
{
    struct mixer_ctl *ctl;

    if (isnumber(name_or_id))
        ctl = mixer_get_ctl(mixer, atoi(name_or_id));
    else
        ctl = mixer_get_ctl_by_name(mixer, name_or_id);

    if (!ctl) {
        fprintf(stderr, "Invalid mixer control\n");
        return;
    }

    print_control_values(ctl);
}

static void print_control_values(struct mixer_ctl *control)
{
    enum mixer_ctl_type type;
    unsigned int num_values;
    unsigned int i;
    int min, max;
    int ret;
    char *buf = NULL;

    type = mixer_ctl_get_type(control);
    num_values = mixer_ctl_get_num_values(control);

    if ((type == MIXER_CTL_TYPE_BYTE) && (num_values > 0)) {
        buf = calloc(1, num_values);
        if (buf == NULL) {
            fprintf(stderr, "Failed to alloc mem for bytes %u\n", num_values);
            return;
        }

        ret = mixer_ctl_get_array(control, buf, num_values);
        if (ret < 0) {
            fprintf(stderr, "Failed to mixer_ctl_get_array\n");
            free(buf);
            return;
        }
    }

    for (i = 0; i < num_values; i++) {
        switch (type)
        {
        case MIXER_CTL_TYPE_INT:
            printf("%d", mixer_ctl_get_value(control, i));
            break;
        case MIXER_CTL_TYPE_BOOL:
            printf("%s", mixer_ctl_get_value(control, i) ? "On" : "Off");
            break;
        case MIXER_CTL_TYPE_ENUM:
            print_enum(control);
            break;
        case MIXER_CTL_TYPE_BYTE:
            printf("%02hhx", buf[i]);
            break;
        default:
            printf("unknown");
            break;
        };
        if ((i + 1) < num_values) {
           printf(", ");
        }
    }

    if (type == MIXER_CTL_TYPE_INT) {
        min = mixer_ctl_get_range_min(control);
        max = mixer_ctl_get_range_max(control);
        printf(" (range %d->%d)", min, max);
    }

    free(buf);
}

static void tinymix_set_byte_ctl(struct mixer_ctl *ctl,
                                 char **values, unsigned int num_values)
{
    int ret;
    char *buf;
    char *end;
    unsigned int i;
    long n;

    buf = calloc(1, num_values);
    if (buf == NULL) {
        fprintf(stderr, "set_byte_ctl: Failed to alloc mem for bytes %u\n", num_values);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < num_values; i++) {
        errno = 0;
        n = strtol(values[i], &end, 0);
        if (*end) {
            fprintf(stderr, "%s not an integer\n", values[i]);
            goto fail;
        }
        if (errno) {
            fprintf(stderr, "strtol: %s: %s\n", values[i],
                strerror(errno));
            goto fail;
        }
        if (n < 0 || n > 0xff) {
            fprintf(stderr, "%s should be between [0, 0xff]\n",
                values[i]);
            goto fail;
        }
        buf[i] = n;
    }

    ret = mixer_ctl_set_array(ctl, buf, num_values);
    if (ret < 0) {
        fprintf(stderr, "Failed to set binary control\n");
        goto fail;
    }

    free(buf);
    return;

fail:
    free(buf);
    exit(EXIT_FAILURE);
}

static int is_int(const char *value)
{
    return value[0] >= '0' && value[0] <= '9';
}

struct parsed_int
{
    /** Wether or not the integer was valid. */
    int valid;
    /** The value of the parsed integer. */
    int value;
    /** The number of characters that were parsed. */
    unsigned int length;
    /** The number of characters remaining in the string. */
    unsigned int remaining_length;
    /** The remaining characters (or suffix) of the integer. */
    const char* remaining;
};

static struct parsed_int parse_int(const char* str)
{
    struct parsed_int out = {
        0 /* valid */,
        0 /* value */,
        0 /* length */,
        0 /* remaining length */,
        NULL /* remaining characters */
    };

    size_t length = strlen(str);
    size_t i = 0;
    int negative = 0;

    if (i < length && str[i] == '-') {
        negative = 1;
        i++;
    }

    while (i < length) {
        char c = str[i++];

        if (c < '0' || c > '9') {
            --i;
            break;
        }

        out.value *= 10;
        out.value += c - '0';

        out.length++;
    }

    if (negative) {
        out.value *= -1;
    }

    out.valid = out.length > 0;
    out.remaining_length = length - out.length;
    out.remaining = str + out.length;

    return out;
}

struct control_value
{
    int value;
    int is_percent;
    int is_relative;
};

static struct control_value to_control_value(const char* value_string)
{
    struct parsed_int parsed_int = parse_int(value_string);

    struct control_value out = {
        0 /* value */,
        0 /* is percent */,
        0 /* is relative */
    };

    out.value = parsed_int.value;

    unsigned int i = 0;

    if (parsed_int.remaining[i] == '%') {
        out.is_percent = 1;
        i++;
    }

    if (parsed_int.remaining[i] == '+') {
        out.is_relative = 1;
    } else if (parsed_int.remaining[i] == '-') {
        out.is_relative = 1;
        out.value *= -1;
    }

    return out;
}

static int set_control_value(struct mixer_ctl* ctl, unsigned int i,
                             const struct control_value* value)
{
    int next_value = value->value;

    if (value->is_relative) {

        int prev_value = value->is_percent ? mixer_ctl_get_percent(ctl, i)
                                           : mixer_ctl_get_value(ctl, i);

        if (prev_value < 0) {
          return prev_value;
        }

        next_value += prev_value;
    }

    return value->is_percent ? mixer_ctl_set_percent(ctl, i, next_value)
                             : mixer_ctl_set_value(ctl, i, next_value);
}

static int set_control_values(struct mixer_ctl* ctl,
                              char** values,
                              unsigned int num_values)
{
    unsigned int num_ctl_values = mixer_ctl_get_num_values(ctl);

    if (num_values == 1) {

        /* Set all values the same */
        struct control_value value = to_control_value(values[0]);

        for (unsigned int i = 0; i < num_values; i++) {
            int res = set_control_value(ctl, i, &value);
            if (res != 0) {
                fprintf(stderr, "Error: invalid value (%d%s%s)\n", value.value,
                        value.is_relative ? "r" : "", value.is_percent ? "%" : "");
                return -1;
            }
        }

    } else {

        /* Set multiple values */
        if (num_values > num_ctl_values) {
            fprintf(stderr,
                    "Error: %u values given, but control only takes %u\n",
                    num_values, num_ctl_values);
            return -1;
        }

        for (unsigned int i = 0; i < num_values; i++) {

            struct control_value v = to_control_value(values[i]);

            int res = set_control_value(ctl, i, &v);
            if (res != 0) {
                fprintf(stderr, "Error: invalid value (%d%s%s) for index %u\n", v.value,
                        v.is_relative ? "r" : "", v.is_percent ? "%" : "", i);
                return -1;
            }
        }
    }

    return 0;
}

static int set_values(struct mixer *mixer, const char *control,
                             char **values, unsigned int num_values)
{
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;

    if (isnumber(control))
        ctl = mixer_get_ctl(mixer, atoi(control));
    else
        ctl = mixer_get_ctl_by_name(mixer, control);

    if (!ctl) {
        fprintf(stderr, "Invalid mixer control\n");
        return -1;
    }

    type = mixer_ctl_get_type(ctl);

    if (type == MIXER_CTL_TYPE_BYTE) {
        tinymix_set_byte_ctl(ctl, values, num_values);
        return 0;
    }

    if (is_int(values[0])) {
        set_control_values(ctl, values, num_values);
    } else {
        if (type == MIXER_CTL_TYPE_ENUM) {
            if (num_values != 1) {
                fprintf(stderr, "Enclose strings in quotes and try again\n");
                return -1;
            }
            if (mixer_ctl_set_enum_by_string(ctl, values[0])) {
                fprintf(stderr, "Error: invalid enum value\n");
                return -1;
            }
        } else {
            fprintf(stderr, "Error: only enum types can be set with strings\n");
            return -1;
        }
    }

    return 0;
}

