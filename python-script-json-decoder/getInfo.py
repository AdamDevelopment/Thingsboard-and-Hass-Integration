import requests
import time
import json
from init import load_variables, thingsboard_auth

def get_all_device_ids(jwt_token, URL, session):
    thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    devices_url = f"{URL}/api/tenant/devices?page=0&pageSize=100"  # Adjust pageSize as needed
    try:
        response = session.get(devices_url, headers=thingsboard_headers)
        response.raise_for_status()
        devices = response.json()
        # Extract device IDs from the response
        device_ids = []
        for device_ids in devices:
            device_ids = devices['data']
        return device_ids
    except requests.exceptions.HTTPError as e:
        print(f"HTTP error occurred: {e}")
        print(f"Response content: {response.content}")
        return []

def getInfo(jwt_token, URL, session, device_id):
    thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    device_url = f"{URL}/api/device/info/{device_id}"  # Adjust the endpoint as needed
    try:
        response = session.get(device_url, headers=thingsboard_headers)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.HTTPError as e:
        print(f"Error fetching data for device {device_id}: {e}")
        return None

def main():
    URL, API_AUTH, USERNAME, PASSWORD, DEVICE_ID, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL = load_variables()
    with requests.Session() as session:
        jwt_token = thingsboard_auth(API_AUTH, USERNAME, PASSWORD, session)
        device_ids = get_all_device_ids(jwt_token, URL, session)
        all_device_data = []
        for device_id in device_ids:
            device_data = getInfo(jwt_token, URL, session, device_id)  # Corrected line
            if device_data:
                all_device_data.append(device_data)
        
        with open('data.json', 'w') as f:
            json.dump(all_device_data, f, indent=4)

if __name__ == '__main__':
    main()




