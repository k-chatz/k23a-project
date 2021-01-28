#!/bin/bash

make clean
make
#valgrind --vgdb=yes --vgdb-error=0 
#valgrind -s --leak-check=full --show-leak-kinds=all  
./project -dir Datasets/camera_specs/2013_camera_specs -csv Datasets/sigmod_large_labelled_dataset.csv -ex resources -m tfidf -sw resources/unwanted-words.txt


echo "Running user main..."

./user -dir Datasets/camera_specs/2013_camera_specs \
-csv resources/datasets/user_dataset.csv \
-vocabulary resources/vocabulary.csv \
-model resources/models/best_model.csv

echo "Done!"
