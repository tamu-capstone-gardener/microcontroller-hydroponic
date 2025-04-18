// sensor_manager.h
#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

void initSensors();
int readMoisture();
int readLightAnalog();
int readLightDigital();
float readTDS();
float readTemperature();
float readHumidity();
void printSensorReadings(int moisture, float temperature, float humidity, int lightAnalog);

#endif
