import requests
import json
import os
from dotenv import find_dotenv, load_dotenv
import datetime

# modify it to get ts from telemetry_response
def data_conv(telemetry_data):
    for key in telemetry_data:
        telemetry_data[key] = telemetry_data[key][0]
    for key, sensors_data in telemetry_data.items():
        if 'ts' and 'value' in sensors_data:
            sensors_data['ts'] = datetime.datetime.fromtimestamp(sensors_data['ts'] / 1000).strftime("%d/%m/%Y %H:%M:%S")
            sensors_data['value'] = float(sensors_data['value'])
                
    
# defining a function for environmental variables
def load_var():
    load_dotenv(find_dotenv())
    URL = os.getenv("url")
    API_AUTH = os.getenv("api_auth")
    USERNAME = os.getenv("name")
    PASSWORD = os.getenv("password")
    DEVICE_ID = os.getenv("device_id")
    FILE_PATH = os.getenv("FILE_PATH")
    return URL, API_AUTH, USERNAME, PASSWORD, DEVICE_ID, FILE_PATH

def save_telemetry_to_json(telemetry_response, file_path):
    try:
        with open(file_path, 'w') as file:
            telemetry_data = telemetry_response.json()
            data_conv(telemetry_data)
            json.dump(telemetry_data, file, indent=2)
    except ValueError as e:
        print("Error saving telemetry data to JSON file:", e)

def get_jwt_token(API_AUTH, USERNAME, PASSWORD, session):
    response = session.post(API_AUTH, json={"username": USERNAME, "password": PASSWORD})
    response.raise_for_status()
    return response.json().get("token")

def telemetry(jwt_token, base_url, DEVICE_ID, FILE_PATH, session):
    if not jwt_token:
        print("Cannot get JWT token.")
        return
    headers = {"X-Authorization": f"Bearer {jwt_token}"}
    telemetry_endpoint = f"{base_url}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries"

    while True:
        try:
            telemetry_response = session.get(telemetry_endpoint, headers=headers)
            telemetry_response.raise_for_status()
            print("Telemetry:", telemetry_response.json())
            save_telemetry_to_json(telemetry_response, FILE_PATH)
        except requests.RequestException as e:
            print("Error retrieving telemetry data:", e)
            break

def main():
    URL, API_AUTH, USERNAME, PASSWORD, DEVICE_ID, FILE_PATH = load_var()
    with requests.Session() as session:
        jwt_token = get_jwt_token(API_AUTH, USERNAME, PASSWORD, session)
        telemetry_response = telemetry(jwt_token, URL, DEVICE_ID, FILE_PATH, session)
        return telemetry_response

if __name__ == "__main__":
    main()
    
