import sys
import subprocess
from pathlib import Path
from datetime import date
from googleapiclient.discovery import build

# TODO: REPLACE THIS API KEY BEFORE ADDING TO GITHUB!!!
api_key = 'AIzaSyBk1k4xQM8NCr-OfUBTVFO_lKQilf6WMHU'

# Create a service from our google api with the service name, version and out api key
service = build('youtube', 'v3', developerKey=api_key)

# -----Find the channel's id from their name-----

# Create a request to get live stream info from a channel
channelRequest = service.channels().list(
    part='snippet',
    id=sys.argv[1]
)
# Execute that request
channelResponse = channelRequest.execute()

channelName = channelResponse["items"][0]["snippet"]["title"]
print(channelName)

# -----Get any running livestreams-----

# Create a request to get live stream info from a channel
liveRequest = service.search().list(
    part='snippet',
    channelId=sys.argv[1],
    type='video',
    eventType='live'
)
# Execute that request
liveResponse = liveRequest.execute()

# -----Set the file name-----

homePath = str(Path.home())
currentDate = date.today().strftime("%d-%m-%Y-")
streamPath = homePath + currentDate + channelName.replace(" ", "-") + ".ts"
print(streamPath)

# -----Start the script that will download the stream-----

try:
    streamId = liveResponse["items"][0]["id"]["videoId"]
    # Print the resulting video id. In the future, this'll be used in the youtube-dl command to capture the livestream
    print(streamId)
    youtubeURL = "https://www.youtube.com/watch?v=" + streamId
    print(youtubeURL)
    proc = subprocess.check_output(["./dl-script.bash", youtubeURL, "/home/jake/Documents/strm.ts"])
    for outLine in proc.splitlines():
        print(outLine)
except:
    print("Stream does not exist or has not started")
