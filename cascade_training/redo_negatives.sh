#! /bin/bash

# Create list of negative images, randomize the list
/bin/find negative_images -name \*.png > negatives.dat
/bin/find negative_images -name \*.jpg >> negatives.dat
shuf negatives.dat > temp.dat
mv temp.dat negatives.dat
