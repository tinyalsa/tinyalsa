#!/bin/sh

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
#   Project configuration variables
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
VERSION_FILE="include/tinyalsa/version.h"
CHANGELOG_FILE="debian/changelog"

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
#   Scripts internal variables
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
LF="\n"
PARAMS=""
DRYRUN=0

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
  echo
  echo "Usage: $0 [OPTIONS] ACTION"
  echo
  echo "Available options:"
  echo "  -s,--script   Format output in \"script\" mode (no trailing newline)."
  echo "  -d,--dry-run  Does not commit anything to any file, just prints."
  echo
  echo "Available actions:"
  echo "  print   [minor|major|patch]  Print the current version."
  echo "  release [minor|major|patch]  Bump the specified version part"
  echo "  check                        Check the changelog latest released"
  echo "                               version against the version file."
  echo
  echo "Please run this script from the project root folder."
  echo
}

check_files()
{
  [ -f ${VERSION_FILE}   ] || die "No ${VERSION_FILE} found!";
  [ -f ${CHANGELOG_FILE} ] || die "No ${CHANGELOG_FILE} found!"
}

# Gets a part of the version from the project version file (version.h).
# Takes one argument: the matching version identifier in the version file, e.g.
#   TINYALSA_VERSION_MAJOR
get_version_part()
{
  set -- "$1" "$(grep -m 1 "^#define\([ \t]*\)$1" ${VERSION_FILE} | sed 's/[^0-9]*//g')"

  if [ -z "$2" ]; then
    die "Could not get $1 from ${VERSION_FILE}"
  fi

  echo "$2"
}


# Gets the complete version from the version file.
# Sets VERSION_MAJOR, VERSION_MINOR and VERSION_PATCH globals
get_version()
{
  VERSION_MAJOR=$(get_version_part "TINYALSA_VERSION_MAJOR")
  VERSION_MINOR=$(get_version_part "TINYALSA_VERSION_MINOR")
  VERSION_PATCH=$(get_version_part "TINYALSA_VERSION_PATCH")
}

# Commits the new version part to the version file.
# Takes two arguments: the version part identifier in the version file and the
#   new version number. If no arguments, do nothing.
commit_version_part()
{
  if [ -z $1 ] || [ -z $2 ]; then
    return 0
  fi

  sed -i "s/\(^#define[ \t]*$1\)[ \t]*\([0-9]*\)/\1 $2/g" ${VERSION_FILE} \
    || die "Could not commit version for $1";

  [ $(get_version_part $1) = "$2" ] || die "Version check after commit failed for $1"

  return 0;
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
  case "${1:-patch}" in
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

  if [ ${DRYRUN} -ne 1 ]; then
    commit_version ${VERSION_PATCH} ${VERSION_MINOR} ${VERSION_MAJOR}
  fi

  print_version
  return 0
}

check_version()
{
  # set $1 to log version, and $2 to ref version
  set -- \
    "$(grep -m 1 "^tinyalsa (" ${CHANGELOG_FILE}| sed "s/[^0-9.]*//g")" \
    "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"

  if [ "$1" != "$2" ]; then
    die "Changelog version ($1) does not match package version ($2)."
  fi

  printf "Changelog version ($1) OK!${LF}"
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
      get_version
      print_version "$2"
      exit $?
      ;;
    release)
      get_version
      bump_version "$2"
      exit $?
      ;;
    check)
      get_version
      check_version
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
    -d|--dry-run)
      DRYRUN=1
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

check_files
parse_command ${PARAMS}

# The script should never reach this place.
die "Internal error. Please report this."
