#!/bin/bash

make clean
make
#valgrind --vgdb=yes --vgdb-error=0 
./project -dir Datasets/camera_specs/2013_camera_specs -csv Datasets/sigmod_large_labelled_dataset.csv -ex resources -m tfidf -sw resources/unwanted-words.txt


echo "Running user main..."

./user -dir resources/user_json_files \
-csv resources/user_dataset.csv \
-vocabulary resources/vocabulary.csv \
-model resources/model.csv

echo "Done!"
