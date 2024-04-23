#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void saveConfigCallback();
void WifiManagerSetup();
void saveConfigFile();
bool loadConfigFile();
void resetDeviceSettings();
void publishAllSensorsData();
void publishAD();
void mqttSetup();
void maxSetup();
void ad8232Setup();
float tempAndHumPublish();
int heartRateDetection();
int32_t MAX30102_SPO2_MEASUREMENT();
#endif // FUNCTIONS_H
