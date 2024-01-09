import requests
import time
from init import load_variables, thingsboard_auth


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
        
def main():
    URL, API_AUTH, USERNAME, PASSWORD, DEVICE_ID, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL, = load_variables()
    with requests.Session() as session:
        jwt_token = thingsboard_auth(API_AUTH, USERNAME, PASSWORD, session)
        telemetry(jwt_token, URL, DEVICE_ID, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL, session)
    
if __name__ == "__main__":
    main()
