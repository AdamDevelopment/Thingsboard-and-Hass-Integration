import os
import requests
import time
from dotenv import load_dotenv, find_dotenv

# Load environment variables
def load_variables():
    load_dotenv(find_dotenv())
    URL = os.getenv("URL")
    API_AUTH = os.getenv("API_AUTH")
    USERNAME = os.getenv("LOGIN")
    PASSWORD = os.getenv("PASSWORD")
    DEVICE_ID = os.getenv("DEVICE_ID")
    HA_TOKEN = os.getenv("HA_TOKEN")
    HA_URL = os.getenv("HA_URL")
    return URL, API_AUTH, USERNAME, PASSWORD, DEVICE_ID, HA_TOKEN, HA_URL

def thingsboard_jwt(API_AUTH, USERNAME, PASSWORD, session):
    response = session.post(API_AUTH, json={"username": USERNAME, "password": PASSWORD})
    response.raise_for_status()
    return response.json().get("token")

def telemetry(jwt_token, URL, DEVICE_ID, HA_TOKEN, HA_URL, session):
    if not jwt_token:
        print("Cannot get JWT token.")
        return
    thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    hass_headers = {'Authorization': f'Bearer {HA_TOKEN}', 'Content-Type': 'application/json'}
    tb_url = f"{URL}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries"
    
    while True:
        try:
            time.sleep(5)
            tb_response = session.get(tb_url, headers=thingsboard_headers)
            tb_response.raise_for_status()
            tb_data = tb_response.json()
            print("Telemetry:", tb_data)
            if 'temperature' in tb_data and 'humidity' in tb_data:
                temperature = tb_data['temperature'][0]['value']
                humidity = tb_data['humidity'][0]['value']
            else:
                temperature = None
                humidity = None
            if temperature is not None:
                ha_temp_url = f"{HA_URL}/api/states/sensor.sht35_temperature"
                ha_temp_data = {
                    "state": temperature,
                    "attributes": {
                        "unit_of_measurement": "°C",
                        "friendly_name": "Czujnik temperatury"
                    }
                }
                ha_temp = requests.post(ha_temp_url, headers=hass_headers, json=ha_temp_data)
                print("HA json:", ha_temp.text)

            if humidity is not None:
                ha_hum_url = f"{HA_URL}/api/states/sensor.sht35_humidity"
                ha_hum_data = {
                    "state": humidity,
                    "attributes": {
                        "unit_of_measurement": "%",
                        "friendly_name": "Czujnik wilgotności"
                    }
                }
                ha_hum = requests.post(ha_hum_url, headers=hass_headers, json=ha_hum_data)
                print("HA json:", ha_hum.text)

        except requests.RequestException as e:
            print("Error:", e)
            break
        
def main():
    URL, API_AUTH, USERNAME, PASSWORD, DEVICE_ID, HA_TOKEN, HA_URL = load_variables()
    with requests.Session() as session:
        jwt_token = thingsboard_jwt(API_AUTH, USERNAME, PASSWORD, session)
        telemetry(jwt_token, URL, DEVICE_ID, HA_TOKEN, HA_URL, session)
    
if __name__ == "__main__":
    main()
