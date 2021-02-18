import json
from googleapiclient.discovery import build

# TODO: REPLACE THIS API KEY BEFORE ADDING TO GITHUB!!!
api_key = 'AIzaSyBk1k4xQM8NCr-OfUBTVFO_lKQilf6WMHU'

# Create a service from our google api with the service name, version and out api key
service = build('youtube', 'v3', developerKey=api_key)

# Create a request to get live stream info from a channel
request = service.search().list(
    part='snippet',
    channelId='UCK9V2B22uJYu3N7eR_BT9QA',
    type='video',
    eventType='live'
)

# Execute that request
response = request.execute()

info = json.dumps(response, indent=4)

# Print the result. It'll be formatted badly but whatever
print(response["items"][0]["id"]["videoId"])
