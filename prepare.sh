#!/bin/bash

rm -rf slides
mkdir -p slides

for file in images/*; do

	name="$(basename "$file" | sed "s/\.j.*//")"
	output="slides/$name"

	convert "$file" -resize 160x128 \
		-background	black \
		-gravity center \
		-extent 160x128 \
		-depth 8 \
		rgb:"$output".raw

done
