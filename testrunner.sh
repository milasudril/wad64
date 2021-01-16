#!/bin/bash
set -e
output_dir="$1/$2"
"$3"
mv *.gcda $output_dir/.
mv *.gcno $output_dir/.