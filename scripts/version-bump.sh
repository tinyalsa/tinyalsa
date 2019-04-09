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
  echo "  -s,--script  Format output in \"script\" mode (no trailing newline)."
  echo
  echo "Available actions:"
  echo "  print   [minor|major|patch]  Print the current version."
  echo "  release [minor|major|patch]  Bump the specified version part"
  echo
  echo "Please run this script from the project root folder."
}


# Gets a part of the version from the project version file (version.h).
# Takes one argument: the matching version identifier in the version file.
get_version_part()
{
  local V=$(grep -m 1 "^#define\([ \t]*\)$1" ${VERSION_FILE} | sed 's/[^0-9]*//g')

  [ ! -z ${V} ] || die "Could not get $1 from ${VERSION_FILE}"

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

# Commits the new version part to the version file.
# Takes two arguments: the version part identifier in the version file and the
#   new version number.
commit_version_part()
{
  if [ -z $1 ] || [ -z $2 ]; then
    return 0
  fi

  sed -i "s/\(^#define[ \t]*$1\)[ \t]*\([0-9]*\)/\1 $2/g" ${VERSION_FILE} \
    || die "Could not commit version";

  return 0
}

# Commits the new version to the version file.
# Takes three arguments, the new version numbers for major, minor and patch
commit_version()
{
  commit_version_part "TINYALSA_VERSION_PATCH" $1
  commit_version_part "TINYALSA_VERSION_MINOR" $2
  commit_version_part "TINYALSA_VERSION_MAJOR" $3

  return 0
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

bump_version()
{
  get_version

  local PART="patch"

  if [ ! -z $1 ]; then
    PART="$1"
  fi

  case "$PART" in
    major)
      VERSION_MAJOR=$((VERSION_MAJOR+1))
      VERSION_MINOR=0
      VERSION_PATCH=0
    ;;
    minor)
      VERSION_MINOR=$((VERSION_MINOR+1))
      VERSION_PATCH=0
    ;;
    patch)
      VERSION_PATCH=$((VERSION_PATCH+1))
    ;;
    *)
      die "Unknown part \"$1\" (must be one of minor, major and patch)."
    ;;
  esac

  commit_version ${VERSION_PATCH} ${VERSION_MINOR} ${VERSION_MAJOR}
  print_version

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
    release)
      bump_version "$2"
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
