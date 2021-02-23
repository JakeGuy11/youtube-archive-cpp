import sys
import requests
import json
import re
from datetime import date

url = sys.argv[1]

req = requests.get(url)
page_data = req.text

needed_line = page_data.splitlines()[19]

needed_json = needed_line.split("ytInitialData = ")[1]
json_string = needed_json.split(";</script>")[0].rstrip()

youtube_data = json.loads(json_string)
video_id = str(youtube_data["contents"]["twoColumnBrowseResultsRenderer"]["tabs"][0]["tabRenderer"]["content"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][0]["channelFeaturedContentRenderer"]["items"][0]["videoRenderer"]["videoId"])
video_url = "https://www.youtube.com/watch?v=" + video_id

video_name = video_id = str(youtube_data["contents"]["twoColumnBrowseResultsRenderer"]["tabs"][0]["tabRenderer"]["content"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][0]["channelFeaturedContentRenderer"]["items"][0]["videoRenderer"]["title"]["runs"][0]["text"])

current_date = date.today().strftime("%d-%m-%Y-") + sys.argv[2]
print(video_url + ";" + current_date)

# print(re.sub(r'\W+', '', video_name))
