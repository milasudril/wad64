#!/bin/bash

prefix=$1
output=$2

cat << thumbnailer > $output
[Thumbnailer Entry]
MimeType=application/x-wad64;
Exec=$prefix/bin/wad64 extract %i:.thumbnail over / as %o
thumbnailer
