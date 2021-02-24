#!/usr/bin/python
# Usage: ./parse_youtube_data.py {CHANNEL_ID}
import sys
import requests
import json
import re
from pathlib import Path
from datetime import date
from fake_useragent import UserAgent

# Generate the channel url from the channel id passed by c++
channel_id = sys.argv[1]
channel_url = "https://www.youtube.com/channel/" + channel_id
try:
    # Declare some fake headers so YouTube doesn't block us
    fake_header = {
        "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9",
        "Accept-Encoding": "gzip, deflate",
        "Accept-Language": "en-GB,en-US;q=0.9,en;q=0.8",
        "Dnt": "1",
        "Upgrade-Insecure-Requests": "1",
        "User-Agent": "Mozilla/5.0 (X11; Linux ppc64le; rv:75.0) Gecko/20100101 Firefox/75.0",
    }
    # Get the source code from the url and pass our fake headers
    req = requests.get(channel_url, headers=fake_header)
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
    # Navigate to where we know the video title will be
    video_name = str(youtube_data["contents"]["twoColumnBrowseResultsRenderer"]["tabs"][0]["tabRenderer"]
                     ["content"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][0][
                     "channelFeaturedContentRenderer"]["items"][0]["videoRenderer"]["title"]["runs"][0]["text"])
    # Generate the title of the saved video from the current date and the user passed title
    return_title = date.today().strftime("%d-%m-%Y") + "-" + re.sub(r'[^a-zA-Z0-9_あ-んア-ン]', '', video_name)
    # Print out a formatted return that the c++ can parse
    print(video_url.rstrip() + ";" + return_title + ".ts")
except:
    print("json error")
