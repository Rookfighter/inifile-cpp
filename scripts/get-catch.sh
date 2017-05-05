#!/bin/bash -e

ARCH="catch.zip"
VERSION="1.9.3"
ARCH_EXTRACT="Catch-$VERSION"
TARGET="catch"
TARGET_DIR="$INSTALL_PREFIX/$TARGET"
DONWLOAD_URL="https://github.com/philsquared/Catch/archive/v$VERSION.zip"

if [ -d $TARGET_DIR ]; then
    echo "-- $TARGET $VERSION already exists"
    return
fi

echo "-- Getting $TARGET $VERSION"
wget -nv --no-check-certificate "$DONWLOAD_URL" -O "$ARCH"
unzip -qq "$ARCH"
mkdir -p "$TARGET"
mv "$ARCH_EXTRACT/single_include/catch.hpp" "$TARGET"
rm -rf "$ARCH" "$ARCH_EXTRACT"
