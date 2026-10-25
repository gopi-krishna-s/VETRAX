# 🔌 VETRAX Hardware & Electrical Wiring Specifications

This document outlines the hardware components, electrical pin mapping, and power distribution network for the **VETRAX Elephant Monitoring & Wildlife Scout Quadruped Robot**.

---

## 🧰 Component Manifest

| Item | Component | Specification / Model | Purpose |
|---|---|---|---|
| **1** | Main Microcontroller | **ESP32 NodeMCU-32S** (Dual-core Xtensa 240MHz, Wi-Fi/BLE) | Locomotion processing, WebServer, GPS parsing |
| **2** | Optical Reconnaissance | **AI-Thinker ESP32-CAM** (OV2640 2MP Camera) | Live elephant video monitoring & surveillance |
| **3** | Satellite Navigation | **u-blox NEO-6M GPS Module** + Ceramic Patch Antenna | Wildlife location tracking & coordinate relay |
| **4** | Actuators (x8) | **MG90S Micro Servos** (Metal Gear, 2.2 kg·cm torque) | 8-DOF quadruped hip and knee joints |
| **5** | Power Source | **7.4V 2S LiPo Battery** (1500–2200 mAh, 25C) | High-current field power supply |
| **6** | Voltage Regulator | **DC-DC Step-Down Buck Converter (UBEC)** (5V / 5A output) | Isolated high-current 5V rail for 8 servos |
| **7** | Logic Power | Direct 5V / USB or secondary 5V 1A LDO | Clean power for ESP32 and GPS module |

---

## ⚡ Direct GPIO Pin Mapping

The VETRAX quadruped drives 8 servos directly via native ESP32 PWM channels:

| Servo Channel | Joint Name | Anatomical Function | ESP32 GPIO Pin | Pulse Width Range | Stance Zero Offset |
|---|---|---|---|---|---|
| **Ch 0** | `RF_HIP` | Right Front Hip Link (`R1`) | **GPIO 4** | 500 – 2400 µs | 90° (Mirrored) |
| **Ch 1** | `RR_HIP` | Right Rear Hip Link (`R2`) | **GPIO 5** | 500 – 2400 µs | 0° (Direct) |
| **Ch 2** | `LF_HIP` | Left Front Hip Link (`L1`) | **GPIO 13** | 500 – 2400 µs | 0° (Direct) |
| **Ch 3** | `LR_HIP` | Left Rear Hip Link (`L2`) | **GPIO 14** | 500 – 2400 µs | 90° (Mirrored) |
| **Ch 4** | `RR_KNEE`| Right Rear Knee/Foot Link (`R4`) | **GPIO 25** | 500 – 2400 µs | 90° (Mirrored) |
| **Ch 5** | `RF_KNEE`| Right Front Knee/Foot Link (`R3`)| **GPIO 26** | 500 – 2400 µs | 0° (Direct) |
| **Ch 6** | `LF_KNEE`| Left Front Knee/Foot Link (`L3`)| **GPIO 27** | 500 – 2400 µs | 90° (Mirrored) |
| **Ch 7** | `LR_KNEE`| Left Rear Knee/Foot Link (`L4`)| **GPIO 33** | 500 – 2400 µs | 0° (Direct) |

---

## 🛰️ GPS Module Wiring (UART2)

| GPS Pin | ESP32 Connection | Description |
|---|---|---|
| **VCC** | **3.3V / 5V** | Power rail (u-blox Neo-6M accepts 3.3V – 5V) |
| **GND** | **GND** | Common ground |
| **TX** | **GPIO 16 (RX2)** | GPS Serial Data Out → ESP32 Hardware UART2 RX |
| **RX** | **GPIO 17 (TX2)** | ESP32 Hardware UART2 TX → GPS Serial Data In |

---

## 🔋 Power Distribution & Noise Isolation

1. **Common Ground:**
   - All ground lines (Battery GND, UBEC GND, ESP32 GND, Servo GND, GPS GND) **must be tied together** to establish a unified reference voltage.
2. **Servo Current Isolation:**
   - **Never power the 8 servos directly from the ESP32 5V/VIN or 3.3V pins.** Eight active micro servos can draw stall currents up to 4.5A, which will cause immediate brownout reboots.
   - Power servos exclusively via the 5V 5A UBEC buck converter.
3. **Decoupling Capacitors:**
   - Place a `1000 µF / 16V` electrolytic capacitor across the servo power rail (`5V` to `GND`) near the servo distribution bus to smooth inductive switching spikes.
4. **Firmware Brownout Mitigation:**
   - In code, brownout detection registers are cleared (`WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0)`) during boot, and servo initialization is staggered by 40ms per channel to eliminate current surges.
