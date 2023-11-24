#!/bin/bash
# Usage ./create_mp4.sh path_to_images (without _XYZA.png) output_file_name
# opts="-r 10 -vb 50M -bt 100M -vf setpts=1.*PTS -pix_fmt yuv420p"
# ffmpeg -i "$1_%04d.png" $opts "out/$2"

ffmpeg -i "flsim/splash_%04d.png" \
	-r 10 -vb 50M -bt 100M -framerate 60 -pix_fmt yuv420p "out/movie.mp4"
