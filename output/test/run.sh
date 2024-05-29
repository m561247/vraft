#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <program> <number_of_times>"
    exit 1
fi

program="$1"
number_of_times="$2"

i=0
while [ $i -lt $number_of_times ]; do
    echo "Running $program for the $((i+1))-th time..."
    $program
    i=$((i+1))
done

echo "Program $program has been run $number_of_times times."