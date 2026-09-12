#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H

#include <Arduino.h>

#define RELAY_PIN 4
#define DEFAULT_AUTOCLOSE_TIME 5000

class LockManager
{
  public:
    LockManager();

    void setup();
    void open();
    void close();
    bool getLockState();
    void setAutocloseTime(unsigned int seconds);
    unsigned int getAutocloseTime();
    void update();

  private:
    bool isOpen;
    unsigned long openPrevMillis;
    unsigned long autoclose_time; 
};

#endif