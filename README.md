# Wearable Fall Detection System for Elderly

A wearable IoT-based fall detection system designed to detect falls in real time using accelerometer and gyroscope data and notify caregivers through the Blynk IoT Cloud platform.

## 📌 Project Overview

This project uses an **ESP32-WROOM-32E** microcontroller and an **MPU6050 (GY-521)** 6-axis accelerometer and gyroscope to monitor the user's movement.

The system calculates the magnitude of acceleration and angular velocity and uses a dual-threshold approach to identify a possible fall. When a fall is detected, a 10-second cancellation window is provided to allow the user to cancel a false alarm. If the alert is not cancelled, the buzzer is activated and a notification is sent to the caregiver through the Blynk mobile application.

## 🔧 Hardware

- ESP32-WROOM-32E
- MPU6050 (GY-521) – 6-axis accelerometer and gyroscope
- SPX3819M5-L-3.3 LDO regulator
- BC817 NPN transistor
- Active buzzer
- Push button
- Battery connector
- CP2102 USB-to-TTL programming interface
- Supporting resistors, capacitors and diodes
- Custom 2-layer PCB

## 💻 Software & Tools

- Arduino IDE
- Arduino Framework (C++)
- Blynk IoT Cloud
- Wokwi
- Altium Designer

## ⚙️ Working Principle

1. The MPU6050 continuously provides accelerometer and gyroscope data to the ESP32 at approximately 21 Hz.
2. The firmware calculates the resultant acceleration and gyroscope magnitudes:

   `AccelMag = √(ax² + ay² + az²)`

   `GyroMag = √(gx² + gy² + gz²)`

3. A fall event is detected when both threshold conditions are satisfied:
   - `AccelMag > 20.0 m/s²`
   - `GyroMag > 3.5 rad/s`

4. When a possible fall is detected, a **10-second cancellation window** is activated.
5. If the user does not cancel the alert:
   - The buzzer is activated.
   - A Blynk event is triggered.
   - A push notification is sent to the caregiver's phone.

## 📡 Blynk IoT

The Blynk platform is used for remote monitoring and alert notification.

The dashboard provides:
- Real-time monitoring of acceleration and gyroscope magnitude
- Push notification when a fall is confirmed

## 🧪 Simulation

The system was simulated using **Wokwi** to verify the operation of the ESP32-based fall detection system.


## 📷 Project Gallery

This repository contains:
- Hardware and PCB images
- Wokwi simulation
- Arduino firmware
- Blynk dashboard screenshots
- Mobile notification results
