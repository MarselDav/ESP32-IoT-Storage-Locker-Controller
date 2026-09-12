#include "impactchecker.h"

ImpactChecker::ImpactChecker(){}

JSONVar ImpactChecker::getStatus()
{
  JSONVar statusJSON;

  if (mpu.getMotionInterruptStatus())
  {
    statusJSON["shock_impact"]["shock_detected"] = true;
  }
  else
  {
    statusJSON["shock_impact"]["shock_detected"] = false;
  }

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  int temperature = temp.temperature;

  if (temperature >= temperature_thresh)
  {
    statusJSON["temperature_impact"]["overheat_detected"] = true;
  }
  else
  {
    statusJSON["temperature_impact"]["overheat_detected"] = false;
  }
  statusJSON["temperature_impact"]["temperature"] = temperature;

  return statusJSON;
}

void ImpactChecker::setTemperatureThresh(int temp_thresh)
{
  this->temperature_thresh = temp_thresh;
}

void ImpactChecker::setup(int thresh_shock, int duration, int temperature_thresh)
{
  this->temperature_thresh = temperature_thresh;
  mpu.begin();
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(thresh_shock);
  mpu.setMotionDetectionDuration(duration);
  mpu.setInterruptPinLatch(true);
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
}