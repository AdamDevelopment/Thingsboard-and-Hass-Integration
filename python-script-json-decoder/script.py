import requests
import json
import time
from init import load_variables, thingsboard_auth

def get_all_device_ids(jwt_token, URL, session):
    thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    devices_url = f"{URL}/api/tenant/devices?page=0&pageSize=100"  # Adjust pageSize as needed
    try:
        response = session.get(devices_url, headers=thingsboard_headers)
        response.raise_for_status()
        devices = response.json()
        # Extract device IDs from the response
        device_ids = [device['id']['id'] for device in devices['data']]
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

def telemetry(jwt_token, URL, DEVICE_ID, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL, session):
    if not jwt_token:
        print("Cannot get JWT token.")
        return
    thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    hass_headers = {'Authorization': f'Bearer {HA_TOKEN}', 'Content-Type': 'application/json'}
    tb_url = f"{URL}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries"
    
    while True:
        try:
            time.sleep(0.1)
            tb_response = session.get(tb_url, headers=thingsboard_headers)
            tb_response.raise_for_status()
            tb_data = tb_response.json()
            print("Telemetry:", tb_data)
            for key in tb_data:
                match key:
                    case 'temperature':
                        temperature = tb_data['temperature'][0]['value']
                        ha_temp_url = HA_TEMP_URL
                        ha_temp_data = {
                            "state": temperature,
                            "attributes": {
                                "unit_of_measurement": "°C",
                                "friendly_name": "Czujnik temperatury"
                            }
                        }
                        ha_temp = requests.post(ha_temp_url, headers=hass_headers, json=ha_temp_data)
                        print("HA json:", ha_temp.text)
                    case 'humidity':
                        humidity = tb_data['humidity'][0]['value']
                        ha_hum_url = HA_HUM_URL
                        ha_hum_data = {
                            "state": humidity,
                            "attributes": {
                                "unit_of_measurement": "%",
                                "friendly_name": "Czujnik wilgotności"
                            }
                        }
                        ha_hum = requests.post(ha_hum_url, headers=hass_headers, json=ha_hum_data)
                        print("HA json:", ha_hum.text)
                    case 'BPM':
                        bpm = tb_data['BPM'][0]['value']
                        ha_bpm_url = HA_BPM_URL
                        ha_bpm_data = {
                            "state": bpm,
                            "attributes": {
                                "unit_of_measurement": "BPM",
                                "friendly_name": "Czujnik tetna"
                            }
                        }
                        ha_bpm = requests.post(ha_bpm_url, headers=hass_headers, json=ha_bpm_data)
                        print("HA json:", ha_bpm.text)
                    case 'ECG':
                        ECG = tb_data['BPM'][0]['value']
                        ha_ecg_url = HA_ECG_URL
                        ha_ecg_data = {
                            "state": ECG,
                            "attributes": {
                                "unit_of_measurement": "ms",
                                "friendly_name": "Czujnik EKG"
                            }
                        }
                        ha_ecg = requests.post(ha_ecg_url, headers=hass_headers, json=ha_ecg_data)
                        print("HA json:", ha_ecg.text)
                    case 'SPO2':
                        SPO2 = tb_data['SPO2'][0]['value']
                        ha_spo2_url = HA_SPO2_URL
                        ha_spo2_data = {
                            "state": SPO2,
                            "attributes": {
                                "unit_of_measurement": "%",
                                "friendly_name": "Czujnik SPO2"
                            }
                        }
                        ha_spo2 = requests.post(ha_spo2_url, headers=hass_headers, json=ha_spo2_data)
                        print("HA json:", ha_spo2.text)
                    case _:
                        print(f"Unknown key: {key}")
        except requests.RequestException as e:
            print("Error:", e)
            break
        

def main_menu():
    print("\nMenu:")
    print("1. List Devices")
    print("2. Add Device")
    print("3. Send Telemetry")
    print("4. Exit")
    return input("Enter your choice: ")

def choose_device(jwt_token, URL, session):
    print("\nAvailable Devices:")
    device_ids = get_all_device_ids(jwt_token, URL, session)
    device_names = []
    for device_id in device_ids:
        device_info = getInfo(jwt_token, URL, session, device_id)
        device_name = device_info.get('name', 'Unknown') if device_info else 'Error fetching name'
        device_names.append(device_name)
        print(f"{device_names.index(device_name) + 1}. {device_name}")

    choice = input("Choose a device to send telemetry to (enter the number): ")
    try:
        selected_index = int(choice) - 1
        if 0 <= selected_index < len(device_names):
            return device_ids[selected_index]
        else:
            print("Invalid selection. Please try again.")
            return None
    except ValueError:
        print("Invalid input. Please enter a number.")
        return None


def create_device(jwt_token, URL, session):
    # # get device profiles
    # thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    # dev_prof_url = f"{URL}/api/deviceProfiles"
    # try:
    #     response = session.get(dev_prof_url, headers=thingsboard_headers)
    #     response.raise_for_status()
    #     dev_prof = response.json()
    #     print("Profiles: ", dev_prof)
    #     for profile in dev_prof:
    #         print(f"- {profile.get('name')}")
    # except requests.exceptions.HTTPError as e:
    #     print(f"Error fetching device profiles: {e}")
    #     return
    
    # create device
    device_name = input("Enter the name of the new device: ")
    device_profile = input("Enter the profile of the new device: ")
    thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    device_data = {
        "name": device_name,
        "type": device_profile
    }
    try:
        response = session.post(f"{URL}/api/device", headers=thingsboard_headers, json=device_data)
        response.raise_for_status()
        created_device = response.json()
        print(f"Device created successfully: {created_device}")
    except requests.exceptions.HTTPError as e:
        print(f"Error creating device: {e}")

def main():
    URL, API_AUTH, USERNAME, PASSWORD, DEVICE_ID, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL = load_variables()
    with requests.Session() as session:
        jwt_token = thingsboard_auth(API_AUTH, USERNAME, PASSWORD, session)
        while True:
            choice = main_menu()
            if choice == '1':
                device_ids = get_all_device_ids(jwt_token, URL, session)
                for device_id in device_ids:
                    device_data = getInfo(jwt_token, URL, session, device_id)
                    print(device_data)
            elif choice == '2':
                create_device(jwt_token, URL, session)
            elif choice == '3':
                selected_device_id = choose_device(jwt_token, URL, session)  # Updated line
                if selected_device_id:
                    telemetry(jwt_token, URL, selected_device_id, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL, session)
            elif choice == '4':
                break
            else:
                print("Invalid choice. Please try again.")


if __name__ == "__main__":
    main()