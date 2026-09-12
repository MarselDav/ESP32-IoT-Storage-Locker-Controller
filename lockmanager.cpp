#include "lockmanager.h"

LockManager::LockManager() : isOpen(false),
openPrevMillis(0), autoclose_time(DEFAULT_AUTOCLOSE_TIME){}

void LockManager::setup()
{
  pinMode(RELAY_PIN, OUTPUT); 
  digitalWrite(RELAY_PIN, LOW);
}

void LockManager::open()
{
  digitalWrite(RELAY_PIN, HIGH);
  openPrevMillis = millis();
  isOpen = true;
}

void LockManager::close()
{
  digitalWrite(RELAY_PIN, LOW);
  isOpen = false;
}

bool LockManager::getLockState()
{
  return isOpen;
}

void LockManager::setAutocloseTime(unsigned int seconds)
{
  autoclose_time = seconds * 1000;
}

unsigned int LockManager::getAutocloseTime()
{
  return autoclose_time / 1000;
}

void LockManager::update()
{
  unsigned long currentMillis = millis();
  if (isOpen && currentMillis - openPrevMillis >= autoclose_time)
  {
    close();
  }
}