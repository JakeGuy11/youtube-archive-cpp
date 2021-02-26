#!/usr/bin/bash

#Check if the script is being run as root, exit if it's not
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

#Get the user's home so we don't use /root/.archive-dl/
USER_HOME=$(getent passwd $SUDO_USER | cut -d: -f6)

#Remove all the directories and files
rm -r "/opt/youtube-archive/"
rm -r "$USER_HOME/.archive-dl/"
rm "/usr/bin/youtube-archive"

echo "Uninstall complete. If you have any feedback, please email me at Jake_Guy_11@protonmail.ch"
