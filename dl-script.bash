#!/bin/bash
echo "Downloading stream $1 into $2"
echo "Line 2 here"

ffmpeg -i `youtube-dl -f best -g $1` $2