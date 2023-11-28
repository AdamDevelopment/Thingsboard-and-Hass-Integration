
# 6.11.2023

- switched to machine states in spo2measurements, to obtain constant measurements in loop() function, without stopping it for 1 second (it was caused by for() loops in primary spo2measurement() function), therefore not being able to measure BPM by heartRateDetection()'s function;</br>
- optimized heartRateDetection()'s function by creating if statement. If IR diode is under 50k, it's sending "0" via MQTT publish function to thingsboard, or sending correct measurements to thingsboard telemetry. Before it, the data shown on thingsboard UI was the last reading from MAX30102, even though there was no finger on it;</br>
- fixed typos and comments;</br>
- Got rid of 'divide by zero' error thrown by ESP32's watchdog, caused by bufforLength calculations in MAX30102's spo2_algorithm. Initialized bufforLength = 50 at INIT case in spo2Measurement()'s function;</br>
- Added esp exception decoder to platformio.ini under monitor_filters:</br>
- Removed unnecessary libraries</br>

# 12.11.2023

-Created working home assistant container</br>

# 14.11.2023

- Sucessfully extracted JSON data via python script from thingsboard</br>

# 18.11.2023

- Created ./python-script-json-decoder folder for python conversion script</br>
- Created .env file to store all sensitive credentials</br>
- Cleaned python code from sensitive data using variables from .env file</br>
- Created timestamp conversion function, to convert ts values from sensors to readable format</br>

# 20.11.2023

- Updated ts_converter(data_conv now) by moving for loops from save_telemetry_to_json() function to data_conv</br>

# 26.11.2023

- Shortened function names</br>

# To-Do

- SSL server's encryption **not needed**</br>
- OTA update via Thingsboard - **read more about it**</br>
- Displaying 0% SpO2 instead of -999% on thingsboard UI, when finger is not detected/placed - **done**</br>
- Create condition to calculate more average BPM in hearRateDetection() function to avoid BPM's spikes - **done**</br>
- !!!Create script to translate JSON payload from thingsboard to one that Home Assistant understand!!!</br>
- If there's time, clean code creating struct, classes etc. (currently at 60-70% of flash memory, so code heavy af)</br>
- After buying electrodes, check if AD8232's payload is making sense (displaying correct ECG function)</br>
- Consider creating noise filters for better readings (AD8232 mostly)</br>
- Somehow create a function to monitor battery voltage</br>
- Measure how long will battery last under full load</br>
- Create functions that will run for desired amount of time, and then going deep-sleep mode, to expand battery life</br>
- Server's backup **done**</br>
- **optional** enable/create automatic server backup</br>
- Read about partitions on ESP32</br>

# To-Do publishing data to Homeassistant

- Correctly set up configuration.yaml in /mnt/c/homeassistant/configuration.yaml</br>
- Correctly add MQTT external tile to Rule chain in Thingsboard and properly configure it</br>
- Create test python function for sending example telemetry</br>
