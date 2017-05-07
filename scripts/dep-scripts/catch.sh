# catch.sh
#
# Created on: 07 May 2017
#     Author: Fabian Meyer
#    License: MIT

export NAME="catch"
export VERSION="1.9.3"
export URL="https://github.com/philsquared/Catch/archive/v$VERSION.zip"
export ARCHIVE="catch.zip"
export ARCHIVE_TYPE=".zip"
export EXTRACT_NAME="Catch-$VERSION"
export EXEC="
    mv \"$NAME/single_include/catch.hpp\" \"/tmp/\" ;
    rm -rf \"$NAME\" ;
    mkdir \"$NAME\" ;
    mv \"/tmp/catch.hpp\" \"$NAME/\""
