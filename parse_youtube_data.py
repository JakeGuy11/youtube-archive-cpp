#!/usr/bin/python
# Usage: ./parse_youtube_data.py {CHANNEL_ID} {STREAM_NICKNAME} {'title' OR 'date'}
import sys
import requests
import json
import re
from datetime import date

# Generate the channel url from the channel id passed by c++
channel_id = sys.argv[1]
channel_url = "https://www.youtube.com/channel/" + channel_id
try:
    # Get the source code from the url
    req = requests.get(channel_url)
    # Convert that source code to a string
    page_data = req.text

    # We know we'll need line 20, so save it as a separate string
    needed_line = page_data.splitlines()[19]

    # We know the preceding and proceeding characters around the json we need, so use them to isolate the json
    needed_json = needed_line.split("ytInitialData = ")[1]
    json_string = needed_json.split(";</script>")[0].rstrip()
except:
    print("network error")

try:
    # Load the json string into a json object
    youtube_data = json.loads(json_string)
    # Navigate to where we know the videoId will be if there is a running stream
    video_id = str(youtube_data["contents"]["twoColumnBrowseResultsRenderer"]["tabs"][0]["tabRenderer"]["content"]
                   ["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][0]
                   ["channelFeaturedContentRenderer"]["items"][0]["videoRenderer"]["videoId"])
    # Add the video id to the default youtube watch url
    video_url = "https://www.youtube.com/watch?v=" + video_id

    # Now we have the video url. Depending on what title the user wants, print out 2 different things
    if sys.argv[3] == "title":
        # Navigate to where we know the video title will be
        video_name = str(youtube_data["contents"]["twoColumnBrowseResultsRenderer"]["tabs"][0]["tabRenderer"]
                         ["content"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][0][
                         "channelFeaturedContentRenderer"]["items"][0]["videoRenderer"]["title"]["runs"][0]["text"])
        # Remove all non-alphanumeric characters so we don't mess up any commands
        return_title = re.sub(r'\W+', '', video_name)
        # Print out a formatted return that the c++ can parse
        print(video_url + ";" + return_title + ".ts")
    else:
        # Generate the title of the saved video from the current date and the user passed title
        return_title = date.today().strftime("%d-%m-%Y-") + sys.argv[2]
        # Print out a formatted return that the c++ can parse
        print(video_url + ";" + return_title + ".ts")
except:
    print("json error")
