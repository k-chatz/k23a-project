#!/bin/bash

make clean && make && valgrind --vgdb=yes --vgdb-error=0 ./project \
-dir Datasets/camera_specs/2013_camera_specs \
-csv Datasets/sigmod_medium_labelled_dataset.csv \
-sw resources/unwanted-words.txt
