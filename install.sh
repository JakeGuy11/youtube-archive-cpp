#!/usr/bin/bash

#Check if the script is being run as root, exit if it's not
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

#Get the user's home so we don't use /root/.archive-dl/
USER_HOME=$(getent passwd $SUDO_USER | cut -d: -f6)

#Make all the directories and files we need to use
mkdir "/opt/youtube-archive/"
mkdir "$USER_HOME/.archive-dl/"
touch "$USER_HOME/.archive-dl/.queue"

#Try to compile the c++ with gcc, and if that doesn't work, use g++
{
gcc -pthread -lstdc++ -g -o youtube-archive youtube-archive.cpp
} || {
g++ -pthread youtube-archive.cpp -g -o youtube-archive
}

#Move all the files that need to be moved
cp ./parse_youtube_data.py /opt/youtube-archive/
cp ./help /opt/youtube-archive/
cp ./youtube-archive /usr/bin/
cp ./youtube-archive.cpp /opt/youtube-archive/

#Ensure the file permissions are correct
chmod +x /usr/bin/youtube-archive
chmod +x /opt/youtube-archive/parse_youtube_data.py

echo "Install is complete, you can delete this folder now."
