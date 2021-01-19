#!/bin/bash

# usage: normbank source_directory destination_directory
# take all .wav file from source_directory, normalize it to -12dB and save result into destination_directory

for filename in $1/*.wav ; do
	name=`basename $filename`
	echo normalizing $1/$name to $2/$name
	sox $1/$name $2/$name norm -12 fade 0.01 0 0.01
done

