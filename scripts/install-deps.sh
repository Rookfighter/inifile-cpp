#!/bin/bash -e

# install-deps.sh
#
# Created on: 07 May 2017
#     Author: Fabian Meyer
#    License: MIT

init_vars()
{
    export NAME=""
    export VERSION=""
    export URL=""
    export ARCHIVE=""
    export ARCHIVE_TYPE=""
    export EXTRACT_NAME=""
    export EXEC=""
}

download_dep()
{
    url="$1"
    archive="$2"
    wget \
        --quiet \
        --no-check-certificate \
        -O "$archive" \
        "$url"
}

extract_archive()
{
    archive="$1"
    archive_type="$2"
    extract_name="$3"
    target_name="$4"

    case $archive_type in
        ".zip")
            unzip -qq "$archive"
            ;;
        ".tar.gz")
            tar xzf "$archive"
            ;;
        *)
            echo "Unsupported archive type $archive_type"
            exit 1
            ;;
    esac

    rm "$archive"
    mv "$extract_name" "$target_name"
}

CURRDIR=$PWD
BASEDIR=$(cd "$(dirname "$0")"; pwd)

# define root install dir for all deps
export INSTALL_PREFIX="$CURRDIR/dep"
# dir where we can find dep files
export DEPSH_ROOT="$BASEDIR/dep-scripts"

echo "Getting dependencies"
mkdir -p "$INSTALL_PREFIX"
cd "$INSTALL_PREFIX"

# get all dep definition files
DEPS=$(ls "$DEPSH_ROOT"/*.sh)
for dep in $DEPS
do
    cd "$INSTALL_PREFIX"
    init_vars
    source "$dep"

    # directory to where dep should be installed
    target_dir="$INSTALL_PREFIX/$NAME"
    if [ -d $target_dir ]; then
        echo "-- $NAME $VERSION already exists"
        continue
    fi

    echo "-- Getting $NAME $VERSION"

    download_dep "$URL" "$ARCHIVE"
    extract_archive "$ARCHIVE" "$ARCHIVE_TYPE" "$EXTRACT_NAME" "$NAME"
    eval $EXEC
done
