# Load environment variables
import os
from dotenv import load_dotenv, find_dotenv

def load_variables():
    load_dotenv(find_dotenv())
    URL = os.getenv("URL")
    API_AUTH = os.getenv("API_AUTH")
    USERNAME = os.getenv("LOGIN")
    PASSWORD = os.getenv("PASSWORD")
    DEVICE_ID = os.getenv("DEVICE_ID", "").split(",")
    HA_TOKEN = os.getenv("HA_TOKEN")
    HA_TEMP_URL = os.getenv("HA_TEMP_URL")
    HA_HUM_URL = os.getenv("HA_HUM_URL")
    HA_BPM_URL = os.getenv("HA_BPM_URL")
    HA_SPO2_URL = os.getenv("HA_SPO2_URL")
    HA_ECG_URL = os.getenv("HA_ECG_URL")
    return URL, API_AUTH, USERNAME, PASSWORD, DEVICE_ID, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL

def thingsboard_auth(API_AUTH, USERNAME, PASSWORD, session):
    response = session.post(API_AUTH, json={"username": USERNAME, "password": PASSWORD})
    response.raise_for_status()
    return response.json().get("token")
