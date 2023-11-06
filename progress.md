

# 6.11.2023
- switched to machine states in spo2measurements, to obtain constant measurements in loop() function, without stopping it for 1 second (it was caused by for() loops in primary spo2measurement() function), therefore not being able to measure BPM by heartRateDetection()'s function;<br>
- optimized heartRateDetection()'s function by creating if statement. If IR diode is under 50k, it's sending "0" via MQTT publish function to thingsboard, or sending correct measurements to thingsboard telemetry. Before it, the data shown on thingsboard UI was the last reading from MAX30102, even though there was no finger on it;<br>
- fixed typos and comments;<br>
- Got rid of 'divide by zero' error thrown by ESP32's watchdog, caused by bufforLength calculations in MAX30102's spo2_algorithm. Initialized bufforLength = 50 at INIT case in spo2Measurement()'s function;<br>
- Added esp exception decoder to platformio.ini under monitor_filters:<br>
- Removed unnecessary libraries<br>

