#!/bin/bash

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
#   Project configuration variables
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
VERSION_FILE="include/tinyalsa/version.h"

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
#   Scripts internal variables
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LF="\n"
PARAMS=""

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
#   Helper functions
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
die()
{
  echo "Error: $@" 1>&2
  exit 1
}

print_usage()
{
  echo "Usage: $0 [OPTIONS] ACTION"
  echo
  echo "Available options:"
  echo "  -s,--script   Format output in \"script\" mode (no trailing newline)."
  echo
  echo "Available actions:"
  echo "  print [minor|major|patch]  Print the current version."
  echo
  echo "Please run this script from the project root folder."
}


# Gets a part of the version from the project version file (version.h).
# Takes one argument: the matching version identifier in the version file.
get_version_part()
{
  local V=$(grep -m 1 "^#define\([ \t]*\)${1}" ${VERSION_FILE} | sed 's/[^0-9]*//g')

  [ ! -z ${V} ] || die "Could not get ${1} from ${VERSION_FILE}"

  echo ${V}
}


# Gets the complete version from the version file.
get_version()
{
  [ -f ${VERSION_FILE} ] || die "No ${VERSION_FILE} found! Is this the project root?";

  VERSION_MAJOR=$(get_version_part "TINYALSA_VERSION_MAJOR")
  VERSION_MINOR=$(get_version_part "TINYALSA_VERSION_MINOR")
  VERSION_PATCH=$(get_version_part "TINYALSA_VERSION_PATCH")
}

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
#   Actions implementations / functions
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
print_version()
{
  get_version

  if [ -z $1 ]; then
    printf "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${LF}"
  else
    case "$1" in
      major)
        printf "${VERSION_MAJOR}${LF}"
        ;;
      minor)
        printf "${VERSION_MINOR}${LF}"
        ;;
      patch)
        printf "${VERSION_PATCH}${LF}"
        ;;
      *)
        die "Unknown part \"$1\" (must be one of minor, major and patch)."
        ;;
    esac
  fi

  return 0
}

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
#   Command Line parsing
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
parse_command()
{
  if [ "$#" -eq "0" ]; then
    print_usage
    exit 1
  fi

  case "$1" in
    print)
      print_version "$2"
      exit $?
      ;;
    *)
      die "Unsupported action \"$1\"."
      ;;
  esac
}

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
#   Main
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

set -e
trap "set +e" 0

# Checking parameters
if [ "$#" -eq "0" ]; then
  print_usage
  exit 0
fi

while [ "$#" -ne "0" ]; do
  case "$1" in
    -s|--script)
      unset LF
      shift
      ;;
    --)
      shift
      break
      ;;
    -*|--*=)
      die "Unsupported flag \"$1\"."
      ;;
    *)
      PARAMS="$PARAMS ${1}"
      shift
      ;;
  esac
done

# set positional arguments in their proper place
set -- "${PARAMS}"

parse_command ${PARAMS}

# The script should never reach this place.
die "Internal error. Please report this."
