import requests
import time
import json
from init import load_variables

def telemetry(jwt_token, URL, DEVICE_ID, session):
    if not jwt_token:
        print("Cannot get JWT token.")
        return
    thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    tb_url = f"{URL}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries"
    
    try:
        tb_response = session.get(tb_url, headers=thingsboard_headers)
        tb_response.raise_for_status()
        tb_data = tb_response.json()
        print("Telemetry:", tb_data)
        # Save telemetry data to a JSON file
        with open('telemetry_data.json', 'w') as json_file:
            json.dump(tb_data, json_file, indent=4)
    except requests.exceptions.HTTPError as e:
        print(f"HTTP error: {e}")
    except requests.exceptions.ConnectionError as e:
        print(f"Connection error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

def main():
    URL, API_AUTH, USERNAME, PASSWORD, DEVICE_ID, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL = load_variables()
    with requests.Session() as session:
        jwt_token = login_auth(API_AUTH, USERNAME, PASSWORD, session)
        if jwt_token:
            telemetry(jwt_token, URL, DEVICE_ID, session)
        else:
            print("Authentication failed.")

def login_auth(API_AUTH, username, password, session):
    response = thingsboard_auth(API_AUTH, json={"username": username, "password": password})
    if response.status_code == 200:
        jwt_token = response.json().get("token")
        print("JWT token:", jwt_token)
        return jwt_token
    else:
        print("Failed to obtain JWT token.")
        return None

if __name__ == "__main__":
    main()
