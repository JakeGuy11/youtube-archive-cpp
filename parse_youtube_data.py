#!/usr/bin/python

import sys
import requests
import json
import re
import time
import random
from pathlib import Path
from datetime import date

# Generate the channel url from the channel id passed by c++
channel_id = sys.argv[1]
channel_url = "https://www.youtube.com/channel/" + channel_id
try:
    user_agents = ["Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.121 Safari/537.36",
                   "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.140 Safari/537.36 Edge/18.17763",
                   "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:54.0) Gecko/20100101 Firefox/54.0",
                   "Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:24.0) Gecko/20100101 Firefox/24.0",
                   "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9a1) Gecko/20070308 Minefield/3.0a1",
                   "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.87 Safari/537.36",
                   "Mozilla/5.0 (X11; Datanyze; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.181 Safari/537.36",
                   "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_6_8) AppleWebKit/534.59.10 (KHTML, like Gecko) Version/5.1.9 Safari/534.59.10",
                   "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10; rv:33.0) Gecko/20100101 Firefox/33.0",
                   "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_6_8) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2623.112 Safari/537.36",
                   "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36",
                   "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3112.113 Safari/537.36",
                   "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.140 Safari/537.36 Edge/18.17763",
                   "Mozilla/5.0 (X11; CrOS x86_64 13421.89.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.183 Safari/537.36",
                   "Mozilla/5.0 (X11; CrOS x86_64 13310.59.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.84 Safari/537.36",
                   "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 YaBrowser/18.3.1.1232 Yowser/2.5 Safari/537.36",
                   "Mozilla/5.0 (iPhone; CPU iPhone OS 10_0 like Mac OS X) AppleWebKit/602.1.50 (KHTML, like Gecko) Version/10.0 YaBrowser/17.6.0.1578.10 Mobile/14A346 Safari/E7FBAF",
                   "Mozilla/5.0 (Windows NT 6.2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.186 YaBrowser/18.3.1.1232 Yowser/2.5 Safari/537.36",
                   "Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:24.0) Gecko/20100101 Firefox/24.0",
                   "BrightSign/8.0.69 (XT1143)Mozilla/5.0 (X11; Linux armv7l) AppleWebKit/537.36 (KHTML, like Gecko) QtWebEngine/5.11.2 Chrome/65.0.3325.230 Safari/537.36",
                   "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:65.0) Gecko/20100101 Firefox/65.0",
                   "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/64.0.3282.24 Safari/537.36",
                   "Mozilla/5.0 (X11; Linux ppc64le; rv:75.0) Gecko/20100101 Firefox/75.0"]
    # Declare some fake headers so YouTube doesn't block us
    fake_header = {
        "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9",
        "Accept-Encoding": "gzip, deflate",
        "Accept-Language": "en-GB,en-US;q=0.9,en;q=0.8",
        "Dnt": "1",
        "Upgrade-Insecure-Requests": "1",
        "User-Agent": random.choice(user_agents)
    }
    # Wait a while so we don't make a bunch of requests immediately and make YouTube suspicious
    time.sleep(2)
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
    return_title = date.today().strftime("%d-%m-%Y") + "-" + sys.argv[2]
    # Print out a formatted return that the c++ can parse
    print(video_url.rstrip() + ";" + return_title + ".ts")
except:
    print("json error")
