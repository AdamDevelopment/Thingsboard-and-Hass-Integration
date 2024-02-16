#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void saveConfigCallback();
void WifiManagerSetup();
void saveConfigFile();
bool loadConfigFile();
void resetDeviceSettings();
void mqttSetup();
void maxSetup();
void ad8232Setup();
void ad8232Publish();
void tempAndHumPublish();
void heartRateDetection();
void MAX30102_SPO2_MEASUREMENT();
void lightSleep();
void getTimeStamp();

#endif // FUNCTIONS_H
