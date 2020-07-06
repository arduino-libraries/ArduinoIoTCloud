#!/bin/bash

if [ "$#" -ne 4 ]; then
  echo "Usage: ota-upload.sh CLIENT_ID CLIENT_SECRET DEVICE_ID sketch.json"
  exit 1
fi

printf "Obtaining JSON Web Token (JWT) ... "
curl --silent --location --request POST 'http://api-dev.arduino.cc/iot/v1/clients/token' \
  --header 'content-type: application/x-www-form-urlencoded' \
  --header 'Content-Type: application/x-www-form-urlencoded' \
  --data-urlencode 'grant_type=client_credentials' \
  --data-urlencode 'client_id='"$1" \
  --data-urlencode 'client_secret='"$2" \
  --data-urlencode 'audience=https://api2.arduino.cc/iot' \
  | jq -r '.access_token' \
  > access_token
if [ -s "access_token" ]; then
  echo "OK"
else
  echo "ERROR"
fi

printf "Uploading to device ... "
access_token_val=$(<access_token)
curl --silent --location --request PUT 'https://api-dev.arduino.cc/iot/v2/devices/'"$3"'/ota' \
  --header 'Content-Type: application/json' \
  --header 'Authorization: Bearer '"$access_token_val" \
  --header 'Content-Type: text/plain' \
  -d @"$4"
echo "OK"

printf "Cleaning up ... "
rm access_token
echo "OK"
