import requests
import time
from init import load_variables, thingsboard_auth
import json

def telemetry(jwt_token, URL, DEVICE_ID, session):
    if not jwt_token:
        print("Cannot get JWT token.")
        return
    thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    tb_url = f"{URL}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries"
    while True:
        try:
            time.sleep(0.1)  # Simulate delay
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
            break

def main():
    URL, API_AUTH, USERNAME, PASSWORD, DEVICE_ID, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL = load_variables()
    with requests.Session() as session:
        jwt_token = thingsboard_auth(API_AUTH, USERNAME, PASSWORD, session)
        if jwt_token:
            telemetry(jwt_token, URL, DEVICE_ID, session)
        else:
            print("Authentication failed.")

if __name__ == "__main__":
    main()
