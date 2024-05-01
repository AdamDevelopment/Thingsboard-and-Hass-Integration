import requests
import time
from tkinter import *
from tkinter import StringVar, Label, Entry
import tkinter as tk
from PIL import Image, ImageTk
from init import load_variables
import threading
from datetime import timezone, datetime

should_continue_telemetry = False

def login_auth(API_AUTH, username, password, session):
    global jwt_token
    response = session.post(API_AUTH, json={"username": username, "password": password})
    if response.status_code == 200:
        jwt_token = response.json().get("token")
        print("JWT token:", jwt_token)
        return True
    else:
        return False



def device_list(jwt_token, URL, session, listbox, device_mapping):
    devices_data = device_names(jwt_token, URL, session)
    listbox.delete(0, tk.END)  
    device_mapping.clear()  

    for name, device_id in devices_data:
        listbox.insert(tk.END, name)  
        device_mapping[name] = device_id  
    
    listbox.after(lambda: device_list(jwt_token, URL, session, listbox, device_mapping))
    
    
def send_telemetry(listbox, device_mapping, jwt_token, URL, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL, window, session):
    try:
        device_name = listbox.get(listbox.curselection())
        device_id = device_mapping.get(device_name)
        if device_id:
            telemetry_thread = threading.Thread(target=telemetry, args=(jwt_token, URL, device_id, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL, session))
            telemetry_thread.start()
        else:
            print("No device selected or device ID not found.")
    except tk.TclError:
        print("No device selected.")

  
def login_handling(username, password, API_AUTH, status_label, window, session):
    username = username.get()
    password = password.get()
    if login_auth(API_AUTH, username, password, session):
        status_label.config(text="Login successful", fg="green")
        window.after(1500, lambda: window.destroy())
    else:
        status_label.config(text="Login failed", fg="red")

 
def loginPanel(API_AUTH, session):
    window = tk.Tk()
    window.title("Thingsboard login panel")
    window.geometry("500x500")
    window.resizable(False, False)
    programLabel = Label(window, text="Thingsboard login panel", font=("Arial Bold", 20))
    programLabel.grid(row=0, column=0)
    programLabel.place(relx=0.5, rely=0.2, anchor=CENTER)
    usernameLabel = Label(window, text="User Name", font=("Arial Bold", 14))
    usernameLabel.grid(row=0, column=0)
    usernameLabel.place(relx=0.3, rely=0.4, anchor=CENTER)
    username = StringVar()
    usernameEntry = Entry(window, textvariable=username, width=30)
    usernameEntry.grid(row=0, column=1, )
    usernameEntry.place(relx=0.6,rely=0.4, anchor=CENTER, height=30)
    passwordLabel = Label(window,text="Password", font=("Arial Bold", 14))
    passwordLabel.grid(row=1, column=0)
    passwordLabel.place(relx=0.3, rely=0.5, anchor=CENTER)
    password= StringVar()
    passwordEntry = Entry(window, textvariable=password, show='*', width=30)
    passwordEntry.grid(row=1, column=1)
    passwordEntry.place(relx=0.6, rely=0.5, anchor=CENTER, height=30)
    status_label = Label(window, text="", font=("Arial Bold", 14))
    status_label.place(relx=0.5, rely=0.7, anchor=tk.CENTER)
    button = tk.Button(window, text="Login", font=("Arial Bold", 15), command=lambda: login_handling(username, password, API_AUTH, status_label, window, session))
    button.grid(row=4, column=0)
    button.place(relx=0.5, rely=0.6, anchor=CENTER, width=80, height=30)
    creatorLabel = Label(text="Created by: AdamDevelopment", font=("Arial Bold", 14))
    creatorLabel.grid(row=6, column=0)
    creatorLabel.place(relx=0.5, rely=0.9, anchor=CENTER)
    window.mainloop()
    
def device_names(jwt_token, URL, session):
    thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    devices_url = f"{URL}/api/tenant/devices?page=0&pageSize=100"
    try:
        response = session.get(devices_url, headers=thingsboard_headers)
        response.raise_for_status()
        devices = response.json()
        devices_data = []
        for device in devices['data']:
            device_name = device.get('name')
            device_id = device['id']['id']
            devices_data.append((device_name, device_id))
        return devices_data
    except requests.exceptions.HTTPError as e:
        print(f"HTTP error occurred: {e}")
        print(f"Response content: {response.content}")
        return []
def telemetry(jwt_token, URL, DEVICE_ID, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL, session):
    if not jwt_token:
        print("Cannot get JWT token.")
        return
    thingsboard_headers = {'X-Authorization': f'Bearer {jwt_token}', 'Content-Type': 'application/json'}
    hass_headers = {'Authorization': f'Bearer {HA_TOKEN}', 'Content-Type': 'application/json'}
    tb_url = f"{URL}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries"
    global should_continue_telemetry
    should_continue_telemetry = True
    while should_continue_telemetry:
        try:
            time.sleep(7)
            current_time_ms = int(datetime.now(tz=timezone.utc).timestamp() * 1000)
            print("Current time:", current_time_ms)
            start_ts = current_time_ms - 14000  # 5 seconds before current time
            print("Start time:", start_ts)
            end_ts = current_time_ms
            print("End time:", end_ts)
            print("Diffrence:", end_ts - start_ts)
            tb_response = session.get(f"{URL}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries", headers=thingsboard_headers)
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
                        tb_url_historical_data = f"{URL}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries?keys=ECG&startTs={start_ts}&endTs={end_ts}&limit=10000&agg=NONE"
                        tb_response_historical = session.get(tb_url_historical_data, headers=thingsboard_headers)
                        tb_response_historical.raise_for_status()
                        tb_data_historical = tb_response_historical.json()
                        print("Historical ECG data:", tb_data_historical)
                        # Preparing data for Home Assistant
                        ha_ecg_data_list = []
                        for sample in reversed(tb_data_historical.get('ECG', [])):
                            timestamp = sample['ts']
                            ECG_value = sample['value']
                            ha_ecg_data_list.append({"timestamp": timestamp, 
                                                     "value": ECG_value})
                        ha_ecg_data = {
                            "state": "OK",
                            "attributes": {
                                "unit_of_measurement": "mV",
                                "friendly_name": "Czujnik EKG",
                                "data": ha_ecg_data_list
                               
                            }
                        }

                        ha_ecg = requests.post(HA_ECG_URL, headers=hass_headers, json=ha_ecg_data)
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
def stop_telemetry():
    global should_continue_telemetry
    should_continue_telemetry = False


def operationalPanel(jwt_token, URL, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL, session):
    window = tk.Tk()
    window.title("Thingsboard operational panel")
    window.geometry("500x500")
    window.resizable(False, False)
    programLabel = Label(window, text="Thingsboard operational panel", font=("Arial Bold", 20))
    programLabel.grid(row=0, column=0)
    programLabel.place(relx=0.5, rely=0.2, anchor=CENTER)
    listboxWindow = tk.Listbox(window, height=3, width=20, selectmode="multiple")
    listboxWindow.grid(row=4, column=0)
    listboxWindow.place(relx=0.5, rely=0.5, anchor=CENTER)
    device_mapping = {}  
    listboxButtonDeviceUpdate = tk.Button(window, text="Update list", font=("Arial Bold", 15), command=lambda: device_list(jwt_token, URL, session, listboxWindow, device_mapping))
    listboxButtonDeviceUpdate.grid(row=5, column=0)
    listboxButtonDeviceUpdate.place(relx=0.5, rely=0.4, anchor=CENTER, width=120, height=30)
    telemetry_button = tk.Button(window, text="Start script", font=("Arial Bold", 15),command=lambda: send_telemetry(listboxWindow, device_mapping, jwt_token, URL, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL, window, session))
    telemetry_button.grid(row=5, column=0)
    telemetry_button.place(relx=0.5, rely=0.6, anchor=CENTER, width=120, height=30)
    telemetry_button_stop = tk.Button(window, text="Stop script", font=("Arial Bold", 15), command=lambda: stop_telemetry())
    telemetry_button_stop.grid(row=6, column=0)
    telemetry_button_stop.place(relx=0.5, rely=0.7, anchor=CENTER, width=120, height=30)
    creatorLabel = Label(text="Wykonał: Adam Błaszczyk, SSIB 236191", font=("Arial Bold", 14))
    creatorLabel.grid(row=6, column=0)
    creatorLabel.place(relx=0.5, rely=0.9, anchor=CENTER)
    window.mainloop()


def main():
    global jwt_token
    with requests.Session() as session:
        URL, API_AUTH, DEVICE_ID, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL = load_variables()
        loginPanel(API_AUTH, session)
        if jwt_token is not None:
            operationalPanel(jwt_token, URL, HA_TOKEN, HA_TEMP_URL, HA_HUM_URL, HA_BPM_URL, HA_SPO2_URL, HA_ECG_URL, session)

if __name__ == "__main__":
    main()
