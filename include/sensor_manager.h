// sensor_manager.h
#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

void initSensors();
int readMoisture();
int readLightAnalog();
int readLightDigital();
float readTemperature();
float readHumidity();

#endif
