#!/bin/bash
# Usage ./create_mp4.sh path_to_images output_file_name
ffmpeg -i $1 \
	-r 30 -vb 50M -bt 100M -vf setpts=1.*PTS -pix_fmt yuv420p $2
