# ESP32 IoT Storage Locker Controller

An embedded IoT access-control system built around an **ESP32 microcontroller**. The controller manages a storage locker, communicates with a mobile application over **MQTT**, monitors environmental and security conditions, and automatically locks the device when required.

The project consists of two parts:

* **ESP32 firmware** — this repository
* **Android mobile application** — [`ESP32-IoT-Controller-App`](https://github.com/MarselDav/ESP32-IoT-Controller-App)

Together, they form an end-to-end IoT system:

```text
┌──────────────────────┐
│   Android App        │
│   Kivy / KivyMD      │
└──────────┬───────────┘
           │
           │ MQTT over TLS
           │
┌──────────▼───────────┐
│       ESP32          │
│                      │
│  ConnectionManager   │
│  LockManager         │
│  ImpactChecker       │
│  BuzzerAlarm         │
└─────┬──────┬─────────┘
      │      │
      │      └──────────────┐
      │                     │
┌─────▼─────┐        ┌──────▼──────┐
│   Relay   │        │   MPU6050   │
│   Lock    │        │   Sensor    │
└───────────┘        └─────────────┘

             ┌─────────────┐
             │   Buzzer    │
             └─────────────┘
```

## Overview

The controller was developed as a university IoT project.

The ESP32 is responsible for the actual operation of the storage locker:

* opening and closing the lock;
* monitoring the lock state;
* detecting physical impact;
* monitoring temperature;
* triggering an alarm;
* automatically closing the lock;
* communicating with the mobile application;
* handling Wi-Fi and MQTT connectivity.

The mobile application provides a user interface for controlling the device and configuring its parameters.

## Features

### Lock Control

The locker is controlled using a relay connected to the ESP32.

The firmware supports:

* opening the lock;
* closing the lock;
* automatic closing after a configurable timeout;
* reporting the current lock state.

The lock control is implemented in the `LockManager` module.

### Automatic Locking

After opening, the controller starts an automatic closing timer.

The timer is implemented using `millis()`, allowing the firmware to perform other tasks while waiting instead of blocking the main loop.

### Impact Detection

An **MPU6050** sensor is used to detect physical impact or attempted tampering.

When an impact exceeding the configured threshold is detected, the controller:

1. triggers the alarm;
2. closes the lock;
3. reports the event through the MQTT connection.

### Temperature Monitoring

The controller monitors the temperature reported by the MPU6050.

If the configured temperature threshold is exceeded, the alarm is triggered and the lock is closed.

### Audible Alarm

A buzzer is used to notify about security events.

The alarm logic is implemented as a non-blocking state machine using `millis()`, allowing the controller to continue processing network and hardware events while the alarm is active.

### MQTT Communication

The ESP32 communicates with the mobile application through an MQTT broker.

The firmware uses:

* `PubSubClient` for MQTT;
* `WiFiClientSecure` for the network connection;
* JSON messages for commands and status data.

The application can send commands to the controller and receive its current state.

### Configuration

The application can configure parameters such as:

* automatic lock timeout;
* temperature threshold;
* status transmission interval.

Configuration changes are transferred to the ESP32 through MQTT messages.

## Communication

The system uses a bidirectional MQTT communication model:

```text
Mobile Application
        │
        │ commands
        ▼
   MQTT Broker
        │
        ▼
      ESP32
        │
        │ status / alarm events
        ▼
   MQTT Broker
        │
        ▼
Mobile Application
```

The controller subscribes to an input topic and publishes status information and events to an output topic.

The MQTT messages contain JSON data describing commands, configuration parameters and controller state.

## Fail-Safe Behavior

The firmware includes several safety-oriented behaviors.

If the ESP32 loses its Wi-Fi or MQTT connection, the lock is closed.

Security events such as:

* physical impact;
* excessive temperature;

also result in the lock being closed and the alarm being activated.

This behavior is intended to keep the storage locker in a closed state when the controller cannot reliably communicate with the rest of the system.

## Firmware Architecture

The firmware is divided into several independent components.

### `ConnectionManager`

Responsible for:

* Wi-Fi connection;
* MQTT connection;
* secure network communication;
* receiving MQTT messages;
* publishing controller state;
* time synchronization.

### `LockManager`

Responsible for:

* relay control;
* lock state;
* automatic closing;
* lock timing.

### `ImpactChecker`

Responsible for:

* MPU6050 communication;
* impact detection;
* temperature monitoring;
* threshold checking.

### `BuzzerAlarm`

Responsible for:

* alarm activation;
* alarm timing;
* non-blocking buzzer control.

### `main.ino`

Coordinates the individual components and contains the main controller loop.

The main loop periodically updates the connection manager, lock manager and alarm system instead of relying on long blocking delays.

## Hardware

The project uses an ESP32-based controller with the following components:

| Component | Purpose                           |
| --------- | --------------------------------- |
| ESP32     | Main microcontroller              |
| Relay     | Electronic lock control           |
| MPU6050   | Impact and temperature monitoring |
| Buzzer    | Audible alarm                     |

The exact hardware wiring depends on the physical implementation of the storage locker.

## Project Structure

```text
IoT/
├── buzzeralarm.h
├── connectionmanager.cpp
├── connectionmanager.h
├── impactchecker.cpp
├── impactchecker.h
├── lockmanager.cpp
├── lockmanager.h
├── main.ino
└── pitches.h
```

The repository is intentionally split into small modules instead of implementing all controller logic in `main.ino`.

## Mobile Application

The ESP32 firmware is designed to work together with the companion Android application.

### [`IoT_APP`](https://github.com/MarselDav/IoT_APP)

The application is implemented in **Python using Kivy/KivyMD** and provides the user interface for the controller.

It communicates with the ESP32 through MQTT and allows the user to:

* control the lock;
* view the current controller state;
* monitor connection status;
* monitor temperature and alarm state;
* configure controller parameters.

The application repository contains the Android application code, Kivy UI definitions and MQTT communication module.

### Complete System

For the complete project, see both repositories:

* **Firmware:** [`MarselDav/IoT`](https://github.com/MarselDav/IoT)
* **Mobile application:** [`MarselDav/IoT_APP`](https://github.com/MarselDav/IoT_APP)

## Technologies

### Embedded

* C++
* Arduino framework
* ESP32
* MPU6050
* Relay control
* Buzzer
* `millis()`-based non-blocking timing

### Networking

* Wi-Fi
* MQTT
* JSON
* `PubSubClient`
* `WiFiClientSecure`

### Mobile Application

* Python
* Kivy
* KivyMD
* Paho MQTT

## Security Notes

The project uses MQTT over a secure network connection.

However, the current educational firmware implementation disables certificate verification for the secure client. Therefore, this implementation should **not** be considered production-grade secure communication.

Before deploying the project in a real environment:

* enable proper broker certificate verification;
* use unique credentials;
* store credentials outside the source code;
* rotate any credentials that may have been exposed during development;
* use appropriate access control and MQTT topic permissions.

The repository should contain only placeholder configuration values rather than real Wi-Fi or MQTT credentials.

## Project Background

This project was developed as a university IoT project.

The goal was to build a complete hardware-software system rather than an isolated microcontroller program. The project combines embedded firmware, sensors, actuators, wireless communication and a mobile control application.

The firmware and mobile application were developed as two parts of the same system.

## What I Learned

This project provided practical experience with:

* ESP32 firmware development;
* embedded C++;
* modular firmware architecture;
* MQTT-based communication;
* JSON message protocols;
* Wi-Fi connectivity;
* secure network connections;
* sensor integration;
* relay and actuator control;
* non-blocking embedded programming;
* event-driven alarm handling;
* mobile-to-device communication;
* designing a complete IoT system across multiple software components.

A particularly important part of the project was coordinating asynchronous events: network connectivity, MQTT messages, sensor events, automatic lock timing and alarm behavior all have to operate without blocking the main controller loop.

## Limitations

This is an educational IoT project rather than a production access-control system.

Some areas that would require additional work for production use include:

* robust certificate management;
* secure credential storage;
* stronger authentication and authorization;
* persistent configuration;
* OTA firmware updates;
* more comprehensive fault handling;
* automated testing;
* production-grade hardware and electrical protection.

## Related Repository

**Mobile application:** [`IoT_APP`](https://github.com/MarselDav/IoT_APP)

**Firmware:** [`IoT`](https://github.com/MarselDav/IoT)
