#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void WifiManagerSetup();
// void wifiSetup();
void mqttSetup();
void maxSetup();
void ad8232Setup();
void tempAndHumPublish();
void heartRateDetection();
void spo2Measurement();
void ad8232Publish();
// void manageDeepSleep(bool enterDeepSleep, unsigned long wakeTime = 5 * 60 * 1000, unsigned long sleepTime = 60 * 1000);
#endif // FUNCTIONS_H
