# 🐘 VETRAX: Autonomous Quadruped Robot for Elephant & Wildlife Corridor Monitoring

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/)
[![Camera: ESP32-CAM](https://img.shields.io/badge/Camera-OV2640-green.svg)](https://ai-thinker.com/)
[![GPS: u--blox_NEO--6M](https://img.shields.io/badge/GPS-u--blox_NEO--6M-orange.svg)](https://www.u-blox.com/)
[![Locomotion: 8--DOF_Quadruped](https://img.shields.io/badge/Locomotion-8--DOF_Quadruped-purple.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**VETRAX** (*Vehicle & Elephant Tracking Autonomous Reconnaissance eXplorer*) is a bio-inspired 8-DOF quadruped scout robot designed for non-invasive wildlife reconnaissance, elephant corridor tracking, and mitigating **Human-Elephant Conflict (HEC)** in rural and forest-fringe regions.

Equipped with an **ESP32 master microcontroller**, **u-blox NEO-6M satellite navigation engine**, an **AI-Thinker ESP32-CAM optical unit with night flash**, and **8 high-torque metal-gear micro servos**, VETRAX navigates rough jungle terrain quietly, logs real-time GPS coordinates, and provides field rangers with an instant mobile-accessible web dashboard and live video feed.

---

## 📑 Table of Contents
- [✨ Key Features](#-key-features)
- [📁 Repository Structure](#-repository-structure)
- [🔌 Hardware Pinout & Wiring](#-hardware-pinout--wiring)
- [🖨️ 3D Printed Mechanical Parts (STLs)](#️-3d-printed-mechanical-parts-stls)
- [🚀 Getting Started & Flashing Firmware](#-getting-started--flashing-firmware)
- [🌐 Web Dashboard & Navigation](#-web-dashboard--navigation)
- [📄 Full Project Report](#-full-project-report)
- [📜 License](#-license)

---

## ✨ Key Features

- **8-DOF Biomimetic Locomotion:** 4-phase diagonal crawl-trot gait tailored for rough forest floors, roots, and uneven vegetation.
- **Satellite Localization:** Onboard u-blox NEO-6M GPS streams latitude, longitude, altitude, speed, satellite count, and direct Google Maps hyperlinks.
- **Optical Surveillance:** High-resolution day/night camera streaming with toggleable high-power Flash LED for night tracking.
- **Zero-Infrastructure Mobile Dashboard:** Built-in Wi-Fi SoftAP web server (`192.168.4.1`) connects directly to any smartphone, tablet, or laptop without needing cellular coverage or internet.
- **Brownout-Proof & Watchdog Protected:** Automated timeout watchdog halts walking if signal is lost, protecting the robot in the wild; staggered servo boot eliminates current spikes.

---

## 📁 Repository Structure

```text
VETRAX/
├── README.md                                # Main project documentation & quickstart
├── VETRAX_PROJECT_REPORT.md                 # In-depth technical research & engineering report
├── docs/
│   ├── HARDWARE_AND_PINOUT.md               # Detailed electrical wiring & power isolation
│   └── LOCOMOTION_AND_GAIT.md               # Quadruped kinematics, diagonal trot maths
├── firmware/
│   ├── esp32_spider_elephant_tracker/       # Arduino IDE sketch (8 Servos + GPS + AP Web UI)
│   │   └── esp32_spider_elephant_tracker.ino
│   ├── esp32cam_elephant_vision/            # ESP32-CAM sketch (MJPEG stream + Night Flash)
│   │   └── esp32cam_elephant_vision.ino
│   └── freertos_esp32_locomotion_gps/       # PlatformIO dual-core FreeRTOS project
│       ├── platformio.ini
│       └── src/main.cpp
└── stl/
    ├── README.md                            # 3D printing guide & slicer parameters
    ├── quadruped_chassis/                   # 11 3D printable parts for primary 8-DOF chassis
    │   ├── Internal-Frame-v121.stl          # Core mounting chassis
    │   ├── Top-Cover-Enclosed-v117.stl      # Protective upper canopy
    │   ├── Bottom-Cover-v121.stl            # Undercarriage skid plate
    │   ├── R1-v117.stl ... R4-v117.stl      # Right leg hip and knee linkages
    │   └── L1-v117.stl ... L4-v117.stl      # Left leg hip and knee linkages
    └── spider_chassis/                      # 22 parts for modular spider chassis
```

---

## 🔌 Hardware Pinout & Wiring

### 1. 8-Servo Direct GPIO Pinout
Servos are driven via native ESP32 PWM timers without requiring an external PCA9685 driver:

| Joint Name | Function | ESP32 GPIO | Pulse Range | Base Zero |
|---|---|---|---|---|
| `RF_HIP` | Right Front Hip Link (`R1`) | **GPIO 4** | 500 – 2400 µs | 90° (Mirrored) |
| `RR_HIP` | Right Rear Hip Link (`R2`) | **GPIO 5** | 500 – 2400 µs | 0° (Direct) |
| `LF_HIP` | Left Front Hip Link (`L1`) | **GPIO 13** | 500 – 2400 µs | 0° (Direct) |
| `LR_HIP` | Left Rear Hip Link (`L2`) | **GPIO 14** | 500 – 2400 µs | 90° (Mirrored) |
| `RR_KNEE`| Right Rear Foot Link (`R4`)| **GPIO 25** | 500 – 2400 µs | 90° (Mirrored) |
| `RF_KNEE`| Right Front Foot Link (`R3`)| **GPIO 26**| 500 – 2400 µs | 0° (Direct) |
| `LF_KNEE`| Left Front Foot Link (`L3`)| **GPIO 27**| 500 – 2400 µs | 90° (Mirrored) |
| `LR_KNEE`| Left Rear Foot Link (`L4`)| **GPIO 33** | 500 – 2400 µs | 0° (Direct) |

### 2. GPS Neo-6M UART2 Connection
- **GPS TX** → **ESP32 GPIO 16 (RX2)**
- **GPS RX** → **ESP32 GPIO 17 (TX2)**
- **VCC** → 3.3V or 5V | **GND** → Common GND

> ⚠️ **IMPORTANT POWER NOTE:**  
> Power the 8 servos exclusively via a high-current **5V 5A DC-DC Buck Converter (UBEC)** powered from a 2S LiPo battery. Do NOT power servos directly from the ESP32 development board pins.

---

## 🖨️ 3D Printed Mechanical Parts (STLs)

All 3D CAD mesh files are located in `stl/quadruped_chassis/` and `stl/spider_chassis/`.
- **Recommended Material:** PETG or ABS/ASA (superior outdoor UV, impact, and heat resistance).
- **Infill:** 30%–40% Gyroid infill.
- **Walls/Perimeters:** 4 perimeters for sturdy screw threading.
- **Hardware:** 8x MG90S metal-gear micro servos, M2 & M3 button head screws.

Detailed print settings and step-by-step mechanical assembly are documented in [`stl/README.md`](stl/README.md).

---

## 🚀 Getting Started & Flashing Firmware

### Method A: Arduino IDE
1. Install the **ESP32 Board Package** in Arduino IDE (`Boards Manager` → `esp32` by Espressif).
2. Install required libraries from `Library Manager`:
   - `ESP32Servo` by Kevin Harrington
   - `TinyGPSPlus` by Mikal Hart
3. Open `firmware/esp32_spider_elephant_tracker/esp32_spider_elephant_tracker.ino`.
4. Select Board: **ESP32 Dev Module** (or **NodeMCU-32S**).
5. Connect your ESP32 via USB and click **Upload**.

### Method B: PlatformIO
```bash
cd firmware/freertos_esp32_locomotion_gps
pio run -t upload
pio devicemonitor -b 115200
```

### Flashing the ESP32-CAM:
1. Open `firmware/esp32cam_elephant_vision/esp32cam_elephant_vision.ino`.
2. Connect FTDI programmer (GPIO 0 connected to GND during boot to enter flashing mode).
3. Select Board: **AI Thinker ESP32-CAM**, set Partition Scheme to **Huge APP (3MB No OTA)**, and upload.

---

## 🌐 Web Dashboard & Navigation

1. Power on the VETRAX robot.
2. On your smartphone or laptop, connect to the Wi-Fi network:
   - **SSID:** `VETRAX_Scout_AP`
   - **Password:** `elephant123`
3. Open your browser and navigate to: **`http://192.168.4.1`**
4. The dashboard displays:
   - **Real-Time GPS Coordinates**: Latitude, Longitude, Altitude, Speed, and Satellites.
   - **Google Maps Button**: One-click opening of the robot's exact coordinates on Google Maps.
   - **D-Pad Directional Controls**: `FWD`, `LEFT`, `STOP`, `RIGHT`, `REV`.
   - **Quick Postures**: `Stand`, `Crouch` (stealth stance), `Sit`.

---

## 📄 Full Project Report

For the complete technical breakdown including kinematics derivations, power consumption curves, conservation impact analysis, and future roadmap (LoRaWAN & Edge AI), read the [**VETRAX Comprehensive Technical Report**](VETRAX_PROJECT_REPORT.md).

---

## 📜 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
