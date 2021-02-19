#!/usr/bin/python

# Grab all needed imports
import sys
import os
import subprocess
from pathlib import Path
from datetime import date
from googleapiclient.discovery import build

# TODO: REPLACE THIS API KEY BEFORE ADDING TO GITHUB!!!
# Open the file with the api key
apiKeyFile = open('.key', mode='r')
# Read the api key to a variable
api_key = apiKeyFile.read()
# Close the api key file
apiKeyFile.close()

# Create a service from our google api with the service name, version and out api key
service = build('youtube', 'v3', developerKey=api_key)

# -----Find the channel info from their id-----

# Create a request to get live stream info from a channel
channelRequest = service.channels().list(
    # Get the a bunch of information (called a snippet)
    part='snippet',
    # Get it for the channel with the specified id
    id=sys.argv[1]
)

# Execute that request
channelResponse = channelRequest.execute()

# For now all we need is the channel name, although we're not using it right now
channelName = channelResponse["items"][0]["snippet"]["title"]
print(channelName)

# -----Get any running livestreams-----

# Create a request to get live stream info from a channel
liveRequest = service.search().list(
    # Get a snippet with the info needed
    part='snippet',
    # Set the channel id to the one put through the command line
    channelId=sys.argv[1],
    # Look for a video
    type='video',
    # Make sure that video is a livestream
    eventType='live'
)

# Execute that request
liveResponse = liveRequest.execute()

# -----Set the file name-----

# Get the home path
homePath = str(Path.home())
# Get the current date and time, DD-MM-YYYY
currentDate = date.today().strftime("%d-%m-%Y-")
# Generate the path to save the stream
streamPath = homePath + "/.holo-dl/" + currentDate + sys.argv[2] + ".ts"
# Print the stream path
print(streamPath)

# -----Start the script that will download the stream-----

try:
    # Get the video id (the part after watch?v=)
    streamId = liveResponse["items"][0]["id"]["videoId"]
    # Print the resulting video id. In the future
    print(streamId)
    # Generate the full video URL
    youtubeURL = "https://www.youtube.com/watch?v=" + streamId
    # Print that URL
    print(youtubeURL)
    # Create and start a process to download the stream
    proc = subprocess.check_output(["./dl-script.bash", youtubeURL, streamPath, sys.argv[2]])
    # Print that process output
    for outLine in proc.splitlines():
        print(outLine)
except:
    # Something went wrong, most likely that the user is not streaming
    print("Stream does not exist or has not started")
