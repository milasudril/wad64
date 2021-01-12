#!/bin/bash
"$3"
output_dir="$1/$2"
mv *.gcda $output_dir/.
mv *.gcno $output_dir/.