
# 6.11.2023

- switched to machine states in spo2measurements, to obtain constant measurements in loop() function, without stopping it for 1 second (it was caused by for() loops in primary spo2measurement() function), therefore not being able to measure BPM by heartRateDetection()'s function;</br>
- optimized heartRateDetection()'s function by creating if statement. If IR diode is under 50k, it's sending "0" via MQTT publish function to thingsboard, or sending correct measurements to thingsboard telemetry. Before it, the data shown on thingsboard UI was the last reading from MAX30102, even though there was no finger on it;</br>
- fixed typos and comments;</br>
- Got rid of 'divide by zero' error thrown by ESP32's watchdog, caused by bufforLength calculations in MAX30102's spo2_algorithm. Initialized bufforLength = 50 at INIT case in spo2Measurement()'s function;</br>
- Added esp exception decoder to platformio.ini under monitor_filters:</br>
- Removed unnecessary libraries</br>

# To-Do

- SSL server's encryption</br>
- OTA update via Thingsboard **read more about it**</br>
- Displaying 0% SpO2 instead of -999% on thingsboard UI, when finger is not detected/placed **done**</br>
- Create condition to calculate more average BPM in hearRateDetection() function to avoid BPM's spikes**done**</br>
- create script to translate JSON payload from thingsboard to one that Home Assistant understand</br>
- If there's time, clean code creating struct, classes etc. (currently at 60-70% of flash memory, so code heavy af)</br>
- After buying electrodes, check if AD8232's payload is making sense (displaying correct ECG function)</br>
- Consider creating noise filters for better readings (AD8232 mostly)</br>
- Somehow create a function to monitor battery voltage</br>
- Measure how long will battery last under full load</br>
- Create functions that will run for desired amount of time, and then going deep-sleep mode, to expand battery life</br>
- Server's backup **done**</br>
