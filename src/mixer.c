/* mixer.c
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <poll.h>

#include <sys/ioctl.h>

#include <linux/ioctl.h>
#define __force
#define __bitwise
#define __user
#include <sound/asound.h>

#include <tinyalsa/mixer.h>

/** A mixer control.
 * @ingroup libtinyalsa-mixer
 */
struct mixer_ctl {
    /** The mixer that the mixer control belongs to */
    struct mixer *mixer;
    /** Information on the control's value (i.e. type, number of values) */
    struct snd_ctl_elem_info info;
    /** A list of string representations of enumerated values (only valid for enumerated controls) */
    char **ename;
};

/** A mixer handle.
 * @ingroup libtinyalsa-mixer
 */
struct mixer {
    /** File descriptor for the card */
    int fd;
    /** Card information */
    struct snd_ctl_card_info card_info;
    /** A continuous array of mixer controls */
    struct mixer_ctl *ctl;
    /** The number of mixer controls */
    unsigned int count;
};

static void mixer_cleanup_control(struct mixer_ctl *ctl)
{
    unsigned int m;

    if (ctl->ename) {
        unsigned int max = ctl->info.value.enumerated.items;
        for (m = 0; m < max; m++)
            free(ctl->ename[m]);
        free(ctl->ename);
    }
}

/** Closes a mixer returned by @ref mixer_open.
 * @param mixer A mixer handle.
 * @ingroup libtinyalsa-mixer
 */
void mixer_close(struct mixer *mixer)
{
    unsigned int n;

    if (!mixer)
        return;

    if (mixer->fd >= 0)
        close(mixer->fd);

    if (mixer->ctl) {
        for (n = 0; n < mixer->count; n++)
            mixer_cleanup_control(&mixer->ctl[n]);
        free(mixer->ctl);
    }

    free(mixer);

    /* TODO: verify frees */
}

static void *mixer_realloc_z(void *ptr, size_t curnum, size_t newnum, size_t size)
{
        int8_t *newp;

        newp = realloc(ptr, size * newnum);
        if (!newp)
            return NULL;

        memset(newp + (curnum * size), 0, (newnum - curnum) * size);
        return newp;
}

static int add_controls(struct mixer *mixer)
{
    struct snd_ctl_elem_list elist;
    struct snd_ctl_elem_id *eid = NULL;
    struct mixer_ctl *ctl;
    int fd = mixer->fd;
    const unsigned int old_count = mixer->count;
    unsigned int new_count;
    unsigned int n;

    memset(&elist, 0, sizeof(elist));
    if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LIST, &elist) < 0)
        goto fail;

    if (old_count == elist.count)
        return 0; /* no new controls return unchanged */

    if (old_count > elist.count)
        return -1; /* driver has removed controls - this is bad */

    ctl = mixer_realloc_z(mixer->ctl, old_count, elist.count,
                          sizeof(struct mixer_ctl));
    if (!ctl)
        goto fail;

    mixer->ctl = ctl;

    /* ALSA drivers are not supposed to remove or re-order controls that
     * have already been created so we know that any new controls must
     * be after the ones we have already collected
     */
    new_count = elist.count;
    elist.space = new_count - old_count; /* controls we haven't seen before */
    elist.offset = old_count; /* first control we haven't seen */

    eid = calloc(elist.space, sizeof(struct snd_ctl_elem_id));
    if (!eid)
        goto fail;

    elist.pids = eid;

    if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LIST, &elist) < 0)
        goto fail;

    for (n = old_count; n < new_count; n++) {
        struct snd_ctl_elem_info *ei = &mixer->ctl[n].info;
        ei->id.numid = eid[n - old_count].numid;
        if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_INFO, ei) < 0)
            goto fail_extend;
        ctl[n].mixer = mixer;
    }

    mixer->count = new_count;
    free(eid);
    return 0;

fail_extend:
    /* cleanup the control we failed on but leave the ones that were already
     * added. Also no advantage to shrinking the resized memory block, we
     * might want to extend the controls again later
     */
    mixer_cleanup_control(&ctl[n]);

    mixer->count = n;   /* keep controls we successfully added */
    /* fall through... */
fail:
    free(eid);
    return -1;
}

/** Opens a mixer for a given card.
 * @param card The card to open the mixer for.
 * @returns An initialized mixer handle.
 * @ingroup libtinyalsa-mixer
 */
struct mixer *mixer_open(unsigned int card)
{
    struct mixer *mixer = NULL;
    int fd;
    char fn[256];

    snprintf(fn, sizeof(fn), "/dev/snd/controlC%u", card);
    fd = open(fn, O_RDWR);
    if (fd < 0)
        return 0;

    mixer = calloc(1, sizeof(*mixer));
    if (!mixer)
        goto fail;

    if (ioctl(fd, SNDRV_CTL_IOCTL_CARD_INFO, &mixer->card_info) < 0)
        goto fail;

    mixer->fd = fd;

    if (add_controls(mixer) != 0)
        goto fail;

    return mixer;

fail:
    if (mixer)
        mixer_close(mixer);
    else if (fd >= 0)
        close(fd);
    return NULL;
}

/** Some controls may not be present at boot time, e.g. controls from runtime
 * loadable DSP firmware. This function adds any new controls that have appeared
 * since mixer_open() or the last call to this function. This assumes a well-
 * behaved codec driver that does not delete controls that already exists, so
 * any added controls must be after the last one we already saw. Scanning only
 * the new controls is much faster than calling mixer_close() then mixer_open()
 * to re-scan all controls.
 *
 * NOTE: this invalidates any struct mixer_ctl pointers previously obtained
 * from mixer_get_ctl() and mixer_get_ctl_by_name(). Either refresh all your
 * stored pointers after calling mixer_update_ctls(), or (better) do not
 * store struct mixer_ctl pointers, instead lookup the control by name or
 * id only when you are about to use it. The overhead of lookup by id
 * using mixer_get_ctl() is negligible.
 * @param mixer An initialized mixer handle.
 * @returns 0 on success, -1 on failure
 */
int mixer_add_new_ctls(struct mixer *mixer)
{
    if (!mixer)
        return 0;

    return add_controls(mixer);
}

/** Gets the name of the mixer's card.
 * @param mixer An initialized mixer handle.
 * @returns The name of the mixer's card.
 * @ingroup libtinyalsa-mixer
 */
const char *mixer_get_name(const struct mixer *mixer)
{
    return (const char *)mixer->card_info.name;
}

/** Gets the number of mixer controls for a given mixer.
 * @param mixer An initialized mixer handle.
 * @returns The number of mixer controls for the given mixer.
 * @ingroup libtinyalsa-mixer
 */
unsigned int mixer_get_num_ctls(const struct mixer *mixer)
{
    if (!mixer)
        return 0;

    return mixer->count;
}

/** Gets the number of mixer controls, that go by a specified name, for a given mixer.
 * @param mixer An initialized mixer handle.
 * @param name The name of the mixer control
 * @returns The number of mixer controls, specified by @p name, for the given mixer.
 * @ingroup libtinyalsa-mixer
 */
unsigned int mixer_get_num_ctls_by_name(const struct mixer *mixer, const char *name)
{
    unsigned int n;
    unsigned int count = 0;
    struct mixer_ctl *ctl;

    if (!mixer)
        return 0;

    ctl = mixer->ctl;

    for (n = 0; n < mixer->count; n++)
        if (!strcmp(name, (char*) ctl[n].info.id.name))
            count++;

    return count;
}

/** Subscribes for the mixer events.
 * @param mixer A mixer handle.
 * @param subscribe value indicating subscribe or unsubscribe for events
 * @returns On success, zero.
 *  On failure, non-zero.
 * @ingroup libtinyalsa-mixer
 */
int mixer_subscribe_events(struct mixer *mixer, int subscribe)
{
    if (ioctl(mixer->fd, SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS, &subscribe) < 0) {
        return -1;
    }
    return 0;
}

/** Wait for mixer events.
 * @param mixer A mixer handle.
 * @param timeout timout value
 * @returns On success, 1.
 *  On failure, -errno.
 *  On timeout, 0
 * @ingroup libtinyalsa-mixer
 */
int mixer_wait_event(struct mixer *mixer, int timeout)
{
    struct pollfd pfd;

    pfd.fd = mixer->fd;
    pfd.events = POLLIN | POLLOUT | POLLERR | POLLNVAL;

    for (;;) {
        int err;
        err = poll(&pfd, 1, timeout);
        if (err < 0)
            return -errno;
        if (!err)
            return 0;
        if (pfd.revents & (POLLERR | POLLNVAL))
            return -EIO;
        if (pfd.revents & (POLLIN | POLLOUT))
            return 1;
    }
}

/** Gets a mixer control handle, by the mixer control's id.
 * For non-const access, see @ref mixer_get_ctl
 * @param mixer An initialized mixer handle.
 * @param id The control's id in the given mixer.
 * @returns A handle to the mixer control.
 * @ingroup libtinyalsa-mixer
 */
const struct mixer_ctl *mixer_get_ctl_const(const struct mixer *mixer, unsigned int id)
{
    if (mixer && (id < mixer->count))
        return mixer->ctl + id;

    return NULL;
}

/** Gets a mixer control handle, by the mixer control's id.
 * For const access, see @ref mixer_get_ctl_const
 * @param mixer An initialized mixer handle.
 * @param id The control's id in the given mixer.
 * @returns A handle to the mixer control.
 * @ingroup libtinyalsa-mixer
 */
struct mixer_ctl *mixer_get_ctl(struct mixer *mixer, unsigned int id)
{
    if (mixer && (id < mixer->count))
        return mixer->ctl + id;

    return NULL;
}

/** Gets the first instance of mixer control handle, by the mixer control's name.
 * @param mixer An initialized mixer handle.
 * @param name The control's name in the given mixer.
 * @returns A handle to the mixer control.
 * @ingroup libtinyalsa-mixer
 */
struct mixer_ctl *mixer_get_ctl_by_name(struct mixer *mixer, const char *name)
{
    return mixer_get_ctl_by_name_and_index(mixer, name, 0);
}

/** Gets an instance of mixer control handle, by the mixer control's name and index.
 *  For instance, if two controls have the name of 'Volume', then and index of 1 would return the second control.
 * @param mixer An initialized mixer handle.
 * @param name The control's name in the given mixer.
 * @param index The control's index.
 * @returns A handle to the mixer control.
 * @ingroup libtinyalsa-mixer
 */
struct mixer_ctl *mixer_get_ctl_by_name_and_index(struct mixer *mixer,
                                                  const char *name,
                                                  unsigned int index)
{
    unsigned int n;
    struct mixer_ctl *ctl;

    if (!mixer)
        return NULL;

    ctl = mixer->ctl;

    for (n = 0; n < mixer->count; n++)
        if (!strcmp(name, (char*) ctl[n].info.id.name))
            if (index-- == 0)
                return mixer->ctl + n;

    return NULL;
}

/** Updates the control's info.
 * This is useful for a program that may be idle for a period of time.
 * @param ctl An initialized control handle.
 * @ingroup libtinyalsa-mixer
 */
void mixer_ctl_update(struct mixer_ctl *ctl)
{
    ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_ELEM_INFO, ctl->info);
}

/** Checks the control for TLV Read/Write access.
 * @param ctl An initialized control handle.
 * @returns On success, non-zero.
 *  On failure, zero.
 * @ingroup libtinyalsa-mixer
 */
int mixer_ctl_is_access_tlv_rw(const struct mixer_ctl *ctl)
{
    return (ctl->info.access & SNDRV_CTL_ELEM_ACCESS_TLV_READWRITE);
}

/** Gets the control's ID.
 * @param ctl An initialized control handle.
 * @returns On success, the control's ID is returned.
 *  On error, UINT_MAX is returned instead.
 * @ingroup libtinyalsa-mixer
 */
unsigned int mixer_ctl_get_id(const struct mixer_ctl *ctl)
{
    if (!ctl)
        return UINT_MAX;

    /* numid values start at 1, return a 0-base value that
     * can be passed to mixer_get_ctl()
     */
    return ctl->info.id.numid - 1;
}

/** Gets the name of the control.
 * @param ctl An initialized control handle.
 * @returns On success, the name of the control.
 *  On error, NULL.
 * @ingroup libtinyalsa-mixer
 */
const char *mixer_ctl_get_name(const struct mixer_ctl *ctl)
{
    if (!ctl)
        return NULL;

    return (const char *)ctl->info.id.name;
}

/** Gets the value type of the control.
 * @param ctl An initialized control handle
 * @returns On success, the type of mixer control.
 *  On failure, it returns @ref MIXER_CTL_TYPE_UNKNOWN
 * @ingroup libtinyalsa-mixer
 */
enum mixer_ctl_type mixer_ctl_get_type(const struct mixer_ctl *ctl)
{
    if (!ctl)
        return MIXER_CTL_TYPE_UNKNOWN;

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:    return MIXER_CTL_TYPE_BOOL;
    case SNDRV_CTL_ELEM_TYPE_INTEGER:    return MIXER_CTL_TYPE_INT;
    case SNDRV_CTL_ELEM_TYPE_ENUMERATED: return MIXER_CTL_TYPE_ENUM;
    case SNDRV_CTL_ELEM_TYPE_BYTES:      return MIXER_CTL_TYPE_BYTE;
    case SNDRV_CTL_ELEM_TYPE_IEC958:     return MIXER_CTL_TYPE_IEC958;
    case SNDRV_CTL_ELEM_TYPE_INTEGER64:  return MIXER_CTL_TYPE_INT64;
    default:                             return MIXER_CTL_TYPE_UNKNOWN;
    };
}

/** Gets the string that describes the value type of the control.
 * @param ctl An initialized control handle
 * @returns On success, a string describing type of mixer control.
 * @ingroup libtinyalsa-mixer
 */
const char *mixer_ctl_get_type_string(const struct mixer_ctl *ctl)
{
    if (!ctl)
        return "";

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:    return "BOOL";
    case SNDRV_CTL_ELEM_TYPE_INTEGER:    return "INT";
    case SNDRV_CTL_ELEM_TYPE_ENUMERATED: return "ENUM";
    case SNDRV_CTL_ELEM_TYPE_BYTES:      return "BYTE";
    case SNDRV_CTL_ELEM_TYPE_IEC958:     return "IEC958";
    case SNDRV_CTL_ELEM_TYPE_INTEGER64:  return "INT64";
    default:                             return "Unknown";
    };
}

/** Gets the number of values in the control.
 * @param ctl An initialized control handle
 * @returns The number of values in the mixer control
 * @ingroup libtinyalsa-mixer
 */
unsigned int mixer_ctl_get_num_values(const struct mixer_ctl *ctl)
{
    if (!ctl)
        return 0;

    return ctl->info.count;
}

static int percent_to_int(const struct snd_ctl_elem_info *ei, int percent)
{
    if ((percent > 100) || (percent < 0)) {
        return -EINVAL;
    }

    int range = (ei->value.integer.max - ei->value.integer.min);

    return ei->value.integer.min + (range * percent) / 100;
}

static int int_to_percent(const struct snd_ctl_elem_info *ei, int value)
{
    int range = (ei->value.integer.max - ei->value.integer.min);

    if (range == 0)
        return 0;

    return ((value - ei->value.integer.min) / range) * 100;
}

/** Gets a percentage representation of a specified control value.
 * @param ctl An initialized control handle.
 * @param id The index of the value within the control.
 * @returns On success, the percentage representation of the control value.
 *  On failure, -EINVAL is returned.
 * @ingroup libtinyalsa-mixer
 */
int mixer_ctl_get_percent(const struct mixer_ctl *ctl, unsigned int id)
{
    if (!ctl || (ctl->info.type != SNDRV_CTL_ELEM_TYPE_INTEGER))
        return -EINVAL;

    return int_to_percent(&ctl->info, mixer_ctl_get_value(ctl, id));
}

/** Sets the value of a control by percent, specified by the value index.
 * @param ctl An initialized control handle.
 * @param id The index of the value to set
 * @param percent A percentage value between 0 and 100.
 * @returns On success, zero is returned.
 *  On failure, non-zero is returned.
 * @ingroup libtinyalsa-mixer
 */
int mixer_ctl_set_percent(struct mixer_ctl *ctl, unsigned int id, int percent)
{
    if (!ctl || (ctl->info.type != SNDRV_CTL_ELEM_TYPE_INTEGER))
        return -EINVAL;

    return mixer_ctl_set_value(ctl, id, percent_to_int(&ctl->info, percent));
}

/** Gets the value of a control.
 * @param ctl An initialized control handle.
 * @param id The index of the control value.
 * @returns On success, the specified value is returned.
 *  On failure, -EINVAL is returned.
 * @ingroup libtinyalsa-mixer
 */
int mixer_ctl_get_value(const struct mixer_ctl *ctl, unsigned int id)
{
    struct snd_ctl_elem_value ev;
    int ret;

    if (!ctl || (id >= ctl->info.count))
        return -EINVAL;

    memset(&ev, 0, sizeof(ev));
    ev.id.numid = ctl->info.id.numid;
    ret = ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_ELEM_READ, &ev);
    if (ret < 0)
        return ret;

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
        return !!ev.value.integer.value[id];

    case SNDRV_CTL_ELEM_TYPE_INTEGER:
        return ev.value.integer.value[id];

    case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
        return ev.value.enumerated.item[id];

    case SNDRV_CTL_ELEM_TYPE_BYTES:
        return ev.value.bytes.data[id];

    default:
        return -EINVAL;
    }

    return 0;
}

/** Gets the contents of a control's value array.
 * @param ctl An initialized control handle.
 * @param array A pointer to write the array data to.
 *  The size of this array must be equal to the number of items in the array
 *  multiplied by the size of each item.
 * @param count The number of items in the array.
 *  This parameter must match the number of items in the control.
 *  The number of items in the control may be accessed via @ref mixer_ctl_get_num_values
 * @returns On success, zero.
 *  On failure, non-zero.
 * @ingroup libtinyalsa-mixer
 */
int mixer_ctl_get_array(const struct mixer_ctl *ctl, void *array, size_t count)
{
    struct snd_ctl_elem_value ev;
    int ret = 0;
    size_t size;
    void *source;
    size_t total_count;

    if ((!ctl) || !count || !array)
        return -EINVAL;

    total_count = ctl->info.count;

    if ((ctl->info.type == SNDRV_CTL_ELEM_TYPE_BYTES) &&
        (mixer_ctl_is_access_tlv_rw(ctl))) {
            /* Additional two words is for the TLV header */
            total_count += TLV_HEADER_SIZE;
    }

    if (count > total_count)
        return -EINVAL;

    memset(&ev, 0, sizeof(ev));
    ev.id.numid = ctl->info.id.numid;

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
    case SNDRV_CTL_ELEM_TYPE_INTEGER:
        ret = ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_ELEM_READ, &ev);
        if (ret < 0)
            return ret;
        size = sizeof(ev.value.integer.value[0]);
        source = ev.value.integer.value;
        break;

    case SNDRV_CTL_ELEM_TYPE_BYTES:
        /* check if this is new bytes TLV */
        if (mixer_ctl_is_access_tlv_rw(ctl)) {
            struct snd_ctl_tlv *tlv;
            int ret;

            if (count > SIZE_MAX - sizeof(*tlv))
                return -EINVAL;
            tlv = calloc(1, sizeof(*tlv) + count);
            if (!tlv)
                return -ENOMEM;
            tlv->numid = ctl->info.id.numid;
            tlv->length = count;
            ret = ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_TLV_READ, tlv);

            source = tlv->tlv;
            memcpy(array, source, count);

            free(tlv);

            return ret;
        } else {
            ret = ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_ELEM_READ, &ev);
            if (ret < 0)
                return ret;
            size = sizeof(ev.value.bytes.data[0]);
            source = ev.value.bytes.data;
            break;
        }

    default:
        return -EINVAL;
    }

    memcpy(array, source, size * count);

    return 0;
}

/** Sets the value of a control, specified by the value index.
 * @param ctl An initialized control handle.
 * @param id The index of the value within the control.
 * @param value The value to set.
 *  This must be in a range specified by @ref mixer_ctl_get_range_min
 *  and @ref mixer_ctl_get_range_max.
 * @returns On success, zero is returned.
 *  On failure, non-zero is returned.
 * @ingroup libtinyalsa-mixer
 */
int mixer_ctl_set_value(struct mixer_ctl *ctl, unsigned int id, int value)
{
    struct snd_ctl_elem_value ev;
    int ret;

    if (!ctl || (id >= ctl->info.count))
        return -EINVAL;

    memset(&ev, 0, sizeof(ev));
    ev.id.numid = ctl->info.id.numid;
    ret = ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_ELEM_READ, &ev);
    if (ret < 0)
        return ret;

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
        ev.value.integer.value[id] = !!value;
        break;

    case SNDRV_CTL_ELEM_TYPE_INTEGER:
        if ((value < mixer_ctl_get_range_min(ctl)) ||
            (value > mixer_ctl_get_range_max(ctl))) {
            return -EINVAL;
        }

        ev.value.integer.value[id] = value;
        break;

    case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
        ev.value.enumerated.item[id] = value;
        break;

    case SNDRV_CTL_ELEM_TYPE_BYTES:
        ev.value.bytes.data[id] = value;
        break;

    default:
        return -EINVAL;
    }

    return ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &ev);
}

/** Sets the contents of a control's value array.
 * @param ctl An initialized control handle.
 * @param array The array containing control values.
 * @param count The number of values in the array.
 *  This must match the number of values in the control.
 *  The number of values in a control may be accessed via @ref mixer_ctl_get_num_values
 * @returns On success, zero.
 *  On failure, non-zero.
 * @ingroup libtinyalsa-mixer
 */
int mixer_ctl_set_array(struct mixer_ctl *ctl, const void *array, size_t count)
{
    struct snd_ctl_elem_value ev;
    size_t size;
    void *dest;
    size_t total_count;

    if ((!ctl) || !count || !array)
        return -EINVAL;

    total_count = ctl->info.count;

    if ((ctl->info.type == SNDRV_CTL_ELEM_TYPE_BYTES) &&
        (mixer_ctl_is_access_tlv_rw(ctl))) {
            /* Additional TLV header */
            total_count += TLV_HEADER_SIZE;
    }

    if (count > total_count)
        return -EINVAL;

    memset(&ev, 0, sizeof(ev));
    ev.id.numid = ctl->info.id.numid;

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
    case SNDRV_CTL_ELEM_TYPE_INTEGER:
        size = sizeof(ev.value.integer.value[0]);
        dest = ev.value.integer.value;
        break;

    case SNDRV_CTL_ELEM_TYPE_BYTES:
        /* check if this is new bytes TLV */
        if (mixer_ctl_is_access_tlv_rw(ctl)) {
            struct snd_ctl_tlv *tlv;
            int ret = 0;
            if (count > SIZE_MAX - sizeof(*tlv))
                return -EINVAL;
            tlv = calloc(1, sizeof(*tlv) + count);
            if (!tlv)
                return -ENOMEM;
            tlv->numid = ctl->info.id.numid;
            tlv->length = count;
            memcpy(tlv->tlv, array, count);

            ret = ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_TLV_WRITE, tlv);
            free(tlv);

            return ret;
        } else {
            size = sizeof(ev.value.bytes.data[0]);
            dest = ev.value.bytes.data;
        }
        break;

    default:
        return -EINVAL;
    }

    memcpy(dest, array, size * count);

    return ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &ev);
}

/** Gets the minimum value of an control.
 * The control must have an integer type.
 * The type of the control can be checked with @ref mixer_ctl_get_type.
 * @param ctl An initialized control handle.
 * @returns On success, the minimum value of the control.
 *  On failure, -EINVAL.
 * @ingroup libtinyalsa-mixer
 */
int mixer_ctl_get_range_min(const struct mixer_ctl *ctl)
{
    if (!ctl || (ctl->info.type != SNDRV_CTL_ELEM_TYPE_INTEGER))
        return -EINVAL;

    return ctl->info.value.integer.min;
}

/** Gets the maximum value of an control.
 * The control must have an integer type.
 * The type of the control can be checked with @ref mixer_ctl_get_type.
 * @param ctl An initialized control handle.
 * @returns On success, the maximum value of the control.
 *  On failure, -EINVAL.
 * @ingroup libtinyalsa-mixer
 */
int mixer_ctl_get_range_max(const struct mixer_ctl *ctl)
{
    if (!ctl || (ctl->info.type != SNDRV_CTL_ELEM_TYPE_INTEGER))
        return -EINVAL;

    return ctl->info.value.integer.max;
}

/** Get the number of enumerated items in the control.
 * @param ctl An initialized control handle.
 * @returns The number of enumerated items in the control.
 * @ingroup libtinyalsa-mixer
 */
unsigned int mixer_ctl_get_num_enums(const struct mixer_ctl *ctl)
{
    if (!ctl)
        return 0;

    return ctl->info.value.enumerated.items;
}

int mixer_ctl_fill_enum_string(struct mixer_ctl *ctl)
{
    struct snd_ctl_elem_info tmp;
    unsigned int m;
    char **enames;

    if (ctl->ename) {
        return 0;
    }

    enames = calloc(ctl->info.value.enumerated.items, sizeof(char*));
    if (!enames)
        goto fail;
    for (m = 0; m < ctl->info.value.enumerated.items; m++) {
        memset(&tmp, 0, sizeof(tmp));
        tmp.id.numid = ctl->info.id.numid;
        tmp.value.enumerated.item = m;
        if (ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_ELEM_INFO, &tmp) < 0)
            goto fail;
        enames[m] = strdup(tmp.value.enumerated.name);
        if (!enames[m])
            goto fail;
    }
    ctl->ename = enames;
    return 0;

fail:
    if (enames) {
        for (m = 0; m < ctl->info.value.enumerated.items; m++) {
            if (enames[m]) {
                free(enames[m]);
            }
        }
        free(enames);
    }
    return -1;
}

/** Gets the string representation of an enumerated item.
 * @param ctl An initialized control handle.
 * @param enum_id The index of the enumerated value.
 * @returns A string representation of the enumerated item.
 * @ingroup libtinyalsa-mixer
 */
const char *mixer_ctl_get_enum_string(struct mixer_ctl *ctl,
                                      unsigned int enum_id)
{
    if (!ctl || (ctl->info.type != SNDRV_CTL_ELEM_TYPE_ENUMERATED) ||
        (enum_id >= ctl->info.value.enumerated.items) ||
        mixer_ctl_fill_enum_string(ctl) != 0)
        return NULL;

    return (const char *)ctl->ename[enum_id];
}

/** Set an enumeration value by string value.
 * @param ctl An enumerated mixer control.
 * @param string The string representation of an enumeration.
 * @returns On success, zero.
 *  On failure, zero.
 * @ingroup libtinyalsa-mixer
 */
int mixer_ctl_set_enum_by_string(struct mixer_ctl *ctl, const char *string)
{
    unsigned int i, num_enums;
    struct snd_ctl_elem_value ev;
    int ret;

    if (!ctl || (ctl->info.type != SNDRV_CTL_ELEM_TYPE_ENUMERATED) ||
        mixer_ctl_fill_enum_string(ctl) != 0)
        return -EINVAL;

    num_enums = ctl->info.value.enumerated.items;
    for (i = 0; i < num_enums; i++) {
        if (!strcmp(string, ctl->ename[i])) {
            memset(&ev, 0, sizeof(ev));
            ev.value.enumerated.item[0] = i;
            ev.id.numid = ctl->info.id.numid;
            ret = ioctl(ctl->mixer->fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &ev);
            if (ret < 0)
                return ret;
            return 0;
        }
    }

    return -EINVAL;
}

