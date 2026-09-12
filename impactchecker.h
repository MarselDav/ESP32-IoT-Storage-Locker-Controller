#ifndef IMPACT_CHECKER_H
#define IMPACT_CHECKER_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Arduino_JSON.h>

class ImpactChecker
{
  public:
    ImpactChecker();

    JSONVar getStatus();
    void setup(int thresh_shock, int duration, int temperature_thresh);
    void setTemperatureThresh(int temp_thresh);

  private:
    Adafruit_MPU6050 mpu;

    int temperature_thresh;
};

#endif