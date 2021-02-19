#!/bin/bash

# Create a var to hold the full file path
filePath="$2"

# Create a file to let the main program know we're already downloading this youtuber's stream
echo "Creating ${filePath%/*}/.$3"
touch "${filePath%/*}/.$3"

# Download the actual stream
ffmpeg -i $(youtube-dl -f best -g "$1") "$2"

# Delete that file to let the main program know we're no longer recording the stream
echo "Deleting ${filePath%/*}/.$3"
rm "${filePath%/*}/.$3"
