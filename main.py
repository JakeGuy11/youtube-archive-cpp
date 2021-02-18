from googleapiclient.discovery import build

api_key = 'AIzaSyBk1k4xQM8NCr-OfUBTVFO_lKQilf6WMHU'

service = build('youtube', 'v3', developerKey=api_key)

request = service.channels().list(
            part='statistics',
            forUsername='schafer5'
        )

response = request.execute()

print(response)

