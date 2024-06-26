
# 6.11.2023

- switched to machine states in spo2measurements, to obtain constant measurements in loop() function, without stopping it for 1 second (it was caused by for() loops in primary spo2measurement() function), therefore not being able to measure BPM by heartRateDetection()'s function;</br>
- optimized heartRateDetection()'s function by creating if statement. If IR diode is under 50k, it's sending "0" via MQTT publish function to thingsboard, or sending correct measurements to thingsboard telemetry. Before it, the data shown on thingsboard UI was the last reading from MAX30102, even though there was no finger on it;</br>
- fixed typos and comments;</br>
- Added esp exception decoder to platformio.ini under monitor_filters:</br>
- Removed unnecessary libraries</br>

# 12.11.2023

-Created working Home Assistant container</br>

# 14.11.2023

- Sucessfully extracted JSON data via python script from thingsboard</br>

# 18.11.2023

- Created ./python-script-json-decoder folder for python conversion script</br>
- Created .env file to store all sensitive credentials</br>
- Cleaned python code from sensitive data using variables from .env file</br>
- Created timestamp conversion function, to convert ts values from sensors to readable format**deprecated**</br>

# 20.11.2023

- Updated ts_converter(data_conv now) by moving for loops from save_telemetry_to_json() function to data_conv**deprecated**</br>

# 26.11.2023

- Shortened function names</br>

# 2.12.2023

- Fixed postgreSQL (compeletly destroyed itself, idk why) and remove unnecessary tables from database</br>
- Changed telementry storing from "never delete" to "7 days", to prevent from taking too much space by it </br>

# 3.12.2023

- added flag to hass-init alias (0.0.0.0:8123) to be able to connect to Docker container via VPN</br>
- Successfully sent telemetry data to Home Assistant's Docker Container via REST API from Thingsboard Server (connected to both Thingsboard REST and Homeassistant REST API's)</br>

# 4.12.2023

- Switched from "if" to "match - case" statements in main loop for better code readability</br>
- Created init.py to clean main code a little bit. Moved load_variables() and thingsboard_auth() functions for better readability and visible separation between initialization functions and main ones</br>

# 10.12.2023

- Moved sensitive credentials to secrets.h</br>

# 13.12.2023

- Insterted code for DeepSleepMode on ESP32 but flash size is too big **in progress**</br>

# 30.12.2023

- Created script.py to add/list existing and new devices, and send telemetry to home assistant's endpoints of choice</br>

# 05.01.2024

- Created WifiManagerSetup() for WifiManager AP and to allow ESP32 to connect to Wifi of user's choice. Also to create a possibility to use ESP32 with diffrent Wifi network, but still be able to send telemetry via MQTT</br>

# 06.03 2024

- Finally an end to this project. It was fun.

# To-Do

- SSL server's encryption **not needed**</br>
- OTA update via Thingsboard - **idea abandoned** </br>
- Displaying 0% SpO2 instead of -999% on thingsboard UI, when finger is not detected/placed - **done**</br>
- Create condition to calculate more average BPM in hearRateDetection() function to avoid BPM's spikes - **done**</br>
- !!!Create script to translate JSON payload from thingsboard to one that Home Assistant understand!!!- **not needed**</br>
- If there's time, clean code creating struct, classes etc. (currently at 60-70% of flash memory, so code heavy ah)</br>
- Somehow create a function to monitor battery voltage - **idea abandoned**</br> 
- Measure how long will battery last under full load - **done**</br>
- Create functions that will run for desired amount of time, and then going deep-sleep mode, to expand battery life - **Not needed**</br>
- Server's backup - **done**</br>
- enable/create automatic server backup - **optional** </br>
- Create nginx server for Home Assistant, get duckdns domain and encrypt it using TLS provided by Let's Encrypt **Not needed**</br>


