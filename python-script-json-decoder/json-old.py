import requests
import json
import os
from dotenv import find_dotenv, load_dotenv
import datetime

# modify it to get ts from telemetry_response
# def ts_converter(ts):
#     return datetime.datetime.fromtimestamp(ts / 10000).isoformat()

def save_telemetry_to_json(telemetry_response, file_path):
    try:
        print("Attempting to save telemetry data...")  # Debugging print
        # Use a raw string for the file path
        with open(file_path, 'w') as file:
            telemetry_data = telemetry_response.json()
            print("Telemetry Data:", telemetry_data)  # Debugging print
            json.dump(telemetry_data, file)
    except IOError as e:
        print("Error saving telemetry data to JSON file:", e)




def get_jwt_token(auth_url, username, password, session):
    response = session.post(auth_url, json={"username": username, "password": password})
    response.raise_for_status()
    return response.json().get("token")

def telemetry(jwt_token, base_url, session):
    if not jwt_token:
        print("Cannot get JWT token.")
        return

    headers = {"X-Authorization": f"Bearer {jwt_token}"}
    telemetry_endpoint = f"{base_url}/api/plugins/telemetry/DEVICE/3003b0a0-6602-11ee-b68b-8d712bbc4e50/values/timeseries"

    while True:
        try:
            telemetry_response = session.get(telemetry_endpoint, headers=headers)
            telemetry_response.raise_for_status()
            print("Telemetry:", telemetry_response.json())
            save_telemetry_to_json(telemetry_response, r'C:\Users\ablas\OneDrive\Dokumenty\PlatformIO\Projects\Engineering Thesis\python-script-json-decoder\telemetry.json')
        except requests.RequestException as e:
            print("Error retrieving telemetry data:", e)
            break

def main():
    url = "https://thingsboardrpi.duckdns.org"
    auth_url = f"{url}/api/auth/login"
    username = "thingsboardrpi@gmail.com"
    password = "B87d6ol19901!"

    with requests.Session() as session:
        jwt_token = get_jwt_token(auth_url, username, password, session)
        telemetry_response = telemetry(jwt_token, url, session)
        return telemetry_response

if __name__ == "__main__":
    main()