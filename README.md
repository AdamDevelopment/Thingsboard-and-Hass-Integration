# Thingsboard and Hass Integration

## ***Introduction***
<p align="justify">
Hello everyone! This is my first ever <strong>"big"</strong> project published on github which is related to my Engineering Thesis. The topic was <strong><em>"Acquisition and monitoring of vital functions of the occupants in the building"</em></strong>. Feel free to explore the code, experiment with it, and provide any feedback or suggestions you might have. I'm open to collaborations and would appreciate any contributions or ideas that could help improve the project. Thank you for checking out my work, and I hope it can be useful or inspiring to you in some way!
</p>

## ***Table of contents***

1. [Overview](#1-overview)
2. [Structure](#2-structure)
3. [Prerequisites](#3-prerequisites)
   - [Software](#software)
   - [Hardware](#hardware)
5. [Getting started](#4-getting-started)

## ***1. Overview***
<p align="justify">  
The main goal of the project was to prepare a system that allows an acquisition and monitoring of vital functions (oxygen saturation, heart rate, temperature and ECG) in a smart building using a dedicated device The element responsible for collecting this data was FireBeetle ESP32-E Iot, while Raspberry Pi 4B was used as an main server. To visualise the collected vital parameters, The thingsboard platform was used as administrative panel for the entire soulution, to which the managing person had remote access, ensuring high system availability and immediate response in case of malfunction. The end system, available to user was Home Assistant, to which data was transmitted using the REST API of the main server. Achieving the flexibility of the solution and its scalabilty was guaranteed by the use of Docker platform, in which previously mentioned buliding automation system was containerized. To secure client – server communication and data transmission via the MQTT protocol, certificates with self – signature were used, ensuring appropriate security of transport layers through SSL/TLS cryptographic protocols.
</p>
<p align="center">
  <img src="image-2.png" alt="Home Assistant user panel">
</p>
<p align="center">
  <img src="image.png" alt="Python login panel for Home Assistant">
</p>

## ***2. Structure***

- `include/`: Contains header files with declarations and macro definitions used across the project
- `python/`: Contains necessary files to authorize user and send collected telemetry data from Thingsboard's database
- `src/`: Contains main C++ files for telemetry aquisition

## ***3. Prerequisites***
> [!NOTE]
>  - Elements listed below are just my subjective choices, which let me to create this project, hence, allowed smooth and effective workflow within my workspace, be free to pick anything that suits you
>  - Learned the hard way, it's better to take a shot with Hass that supports HACS (Home Assistant Community Store).

### Software
- Visual Studio Code
- PlatformIO extension
- Docker
- Thingsboard Community Edition
- Home Assistant (Core)

### Hardware
- Raspberry Pi 4B 8GB
- FireBeetle 2 ESP32-E IoT
- MAX30102 (pulse and saturation)
- AD8232 (ECG)
- SHT35 (temperature)

## ***4. Getting Started***

1. Clone the repository: <https://github.com/AdamDevelopment/Thingsboard-and-Hass-Integration.git>
2. Navigate into the repository directory:
cd Thingsboard-and-Hass-Integration

3. Follow the setup instructions for PlatformIO and integrate the necessary libraries:

- Wire
- knolleary/PubSubClient@^2.8
- <https://github.com/Risele/SHT3x.git>
- sparkfun/SparkFun MAX3010x Pulse and Proximity Sensor Library@^1.1.2
- <https://github.com/tzapu/WiFiManager>
- <https://github.com/bblanchon/ArduinoJson>

## ***5. Contributing***

Contributions are welcome! Please feel free to submit pull requests or open issues to suggest improvements or report bugs.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
