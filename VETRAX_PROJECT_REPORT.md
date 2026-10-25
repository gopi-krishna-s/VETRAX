# 🐘 VETRAX: Autonomous Quadruped Robot for Elephant Corridor Monitoring & Human-Wildlife Conflict Mitigation
### Comprehensive Engineering Research & Technical Project Report
**Author / Developer:** Gopi Krishna S.  
**Repository:** [https://github.com/gopi-krishna-s/VETRAX.git](https://github.com/gopi-krishna-s/VETRAX.git)  
**System Hardware:** ESP32 NodeMCU-32S, AI-Thinker ESP32-CAM, u-blox NEO-6M GPS, 8x Metal-Gear Micro Servos, 3D Printed 8-DOF Biomimetic Quadruped Chassis  

---

## 1. Executive Summary

Human-Elephant Conflict (HEC) represents one of the most critical conservation and rural safety crises in tropical forest fringes across Asia and Africa. Unmonitored elephant herds traversing agricultural zones, railway tracks, and human settlements frequently result in crop destruction, infrastructure damage, and catastrophic loss of human and wildlife life. Traditional static sensor fencing and drone patrols suffer from dense jungle canopy obscuration, high operational costs, battery limitations, and acoustic disturbance that agitates wild herds.

**VETRAX (Vehicle & Elephant Tracking Autonomous Reconnaissance eXplorer)** is a low-profile, bio-inspired 8-DOF quadruped scout robot designed for quiet, non-invasive ground-level reconnaissance in forested corridors and agricultural buffer zones. Equipped with an **ESP32 dual-core microcontroller**, **u-blox NEO-6M satellite navigation engine**, an **ESP32-CAM optical video surveillance unit with night flash**, and **8 high-torque metal-gear micro servos**, VETRAX traverses rugged forest floor terrain, logs high-precision geographic coordinates, and streams real-time telemetry and imagery directly to field rangers and monitoring stations via a standalone Wi-Fi access point web dashboard.

---

## 2. Problem Statement & Operational Objectives

### 2.1 The Challenge
1. **Canopy Blind Spots:** Aerial drones cannot penetrate dense forest canopies where elephant herds seek daytime shelter.
2. **Terrain Inaccessibility:** Wheeled and tracked rovers get trapped in mud, dense brush, undergrowth, and rocky riverbeds.
3. **Acoustic Agitation:** Loud motorized vehicles or drones trigger defensive stampedes in elephant herds.
4. **Real-Time Localization:** Rangers require instantaneous GPS coordinates linked directly to mapping interfaces to dispatch early-warning deterrents (e.g., acoustic sirens, light beacons).

### 2.2 VETRAX Design Objectives
- **Bio-Inspired Locomotion:** 8-DOF quadrupedal structure with crawl/trot gaits capable of stepping over vegetation, roots, and uneven terrain.
- **Silent & Stealthy Stance:** Low-profile crouch postures (`CROUCH` / `SIT`) allowing quiet observation without alarming wild animals.
- **Autonomous GPS Localization:** Continuous satellite positioning via hardware UART feeding real-time latitude, longitude, speed, and altitude to field personnel.
- **Dual Optical Surveillance:** Day and night visual verification of herd count, calves, and direction of travel via ESP32-CAM with integrated flash illumination.
- **Zero Infrastructure Dependency:** Onboard Wi-Fi SoftAP web server functioning completely off-grid without requiring cellular towers or internet connection.
- **Brownout-Proof & Failsafe Architecture:** Dynamic watchdog timers and hardware power isolation preventing field immobilization.

---

## 3. System Architecture

The VETRAX architecture integrates mechanical, electrical, firmware, and telemetry subsystems into a unified field-deployable explorer:

```text
+-------------------------------------------------------------------------+
|                              VETRAX ROBOT                               |
|                                                                         |
|  +--------------------+                     +------------------------+  |
|  |  AI-Thinker        |                     |  ESP32 NodeMCU-32S     |  |
|  |  ESP32-CAM         |                     |  (Master Controller)   |  |
|  |                    |                     |                        |  |
|  | - OV2640 2MP Camera|  Local Wi-Fi Link   | - Dual Core 240MHz     |  |
|  | - MJPEG Streamer   |<------------------->| - SoftAP Web Server    |  |
|  | - Night Flash LED  |                     | - Watchdog Failsafe    |  |
|  +--------------------+                     +-----------+------------+  |
|                                                         |               |
|                                           UART2 (16/17) | PWM (8x GPIO) |
|                                                         v               v
|                                               +---------+---+  +--------+---+
|                                               | NEO-6M GPS  |  | 8x Servos  |
|                                               | Sat Nav     |  | 8-DOF Legs |
|                                               +-------------+  +------------+
+-------------------------------------------------------------------------+
                                      |
                            Wi-Fi AP (192.168.4.1)
                                      v
                        +----------------------------+
                        | Mobile / Field Ranger UI   |
                        | - Directional D-Pad        |
                        | - Live Google Maps GPS Pin |
                        | - Live Video Stream Feed   |
                        +----------------------------+
```

---

## 4. Hardware & Electrical Specifications

### 4.1 Component Breakdown

| Subsystem | Component | Parameters | Role in VETRAX |
|---|---|---|---|
| **Brain / Processing** | ESP32 NodeMCU-32S | Xtensa LX6 Dual-Core @ 240MHz, 520KB SRAM, 4MB Flash | Gait generation, WebServer host, GPS parser |
| **Vision Unit** | AI-Thinker ESP32-CAM | OV2640 2MP Sensor, High-Power Flash LED | Wildlife visual reconnaissance, night tracking |
| **Positioning** | u-blox NEO-6M GPS | 50 Channels, 2.5m CEP horizontal accuracy, Update rate 1–5Hz | Continuous latitude/longitude satellite telemetry |
| **Actuation (x8)** | TowerPro MG90S | Metal gears, 2.2 kg·cm stall torque @ 4.8V, 0.10s/60° | 4 Hip yaw/pitch joints + 4 Knee pitch joints |
| **Power Storage** | 2S LiPo Battery Pack | 7.4V nominal, 2200 mAh, 25C continuous discharge | High current supply for peak servo loads |
| **Power Regulation** | DC-DC Buck Converter (UBEC) | Input 7.4V, Output 5V @ 5A continuous | Clean, high-amperage power rail isolated for servos |
| **Antenna** | Active Ceramic Patch Antenna | 25x25mm, L1 frequency (1575.42 MHz) | High sensitivity GPS signal lock under trees |

### 4.2 Pin Allocation Matrix

| Channel | Joint ID | Physical Leg Link | ESP32 GPIO | Logic Function | Neutral Trim |
|---|---|---|---|---|---|
| **Ch 0** | `RF_HIP` | Right Front Hip (`R1`) | **GPIO 4** | LEDC PWM Timer 0 | 90° (Mirrored) |
| **Ch 1** | `RR_HIP` | Right Rear Hip (`R2`) | **GPIO 5** | LEDC PWM Timer 0 | 0° (Direct) |
| **Ch 2** | `LF_HIP` | Left Front Hip (`L1`) | **GPIO 13** | LEDC PWM Timer 1 | 0° (Direct) |
| **Ch 3** | `LR_HIP` | Left Rear Hip (`L2`) | **GPIO 14** | LEDC PWM Timer 1 | 90° (Mirrored) |
| **Ch 4** | `RR_KNEE`| Right Rear Foot (`R4`) | **GPIO 25** | LEDC PWM Timer 2 | 90° (Mirrored) |
| **Ch 5** | `RF_KNEE`| Right Front Foot (`R3`)| **GPIO 26** | LEDC PWM Timer 2 | 0° (Direct) |
| **Ch 6** | `LF_KNEE`| Left Front Foot (`L3`)| **GPIO 27** | LEDC PWM Timer 3 | 90° (Mirrored) |
| **Ch 7** | `LR_KNEE`| Left Rear Foot (`L4`)| **GPIO 33** | LEDC PWM Timer 3 | 0° (Direct) |
| **UART2 RX**| `GPS_RX` | GPS Module TX Pin | **GPIO 16** | Hardware Serial 2 RX | 9600 Baud |
| **UART2 TX**| `GPS_TX` | GPS Module RX Pin | **GPIO 17** | Hardware Serial 2 TX | 9600 Baud |

---

## 5. Mechanical Engineering & 3D Printable STL Assets

The robot's chassis is 3D printed using high-durability polymer filaments designed to absorb dynamic trail vibrations and resist moisture. All STL files are categorized in the `stl/` directory:

### 5.1 Primary Quadruped Parts (`stl/quadruped_chassis/`)
1. **`Internal-Frame-v121.stl`**: Central skeletal core featuring mounting bays for the ESP32, GPS breakout, 2S LiPo battery strap, and integrated cable routing conduits.
2. **`Top-Cover-Enclosed-v117.stl`**: Canopy shell shielding electronic components from dew, falling leaves, and debris.
3. **`Bottom-Cover-v121.stl`**: Rugged belly skid plate protecting wiring from sharp rocks and damp terrain.
4. **`R1-v117.stl` & `R2-v117.stl`**: Right-side front and rear hip linkages housing the MG90S coxa servos.
5. **`L1-v117.stl` & `L2-v117.stl`**: Left-side front and rear hip linkages.
6. **`R3-v117.stl` & `R4-v117.stl`**: Right-side front and rear knee-foot linkages.
7. **`L3-v117.stl` & `L4-v117.stl`**: Left-side front and rear knee-foot linkages.

### 5.2 Print & Material Specifications
- **Recommended Material:** **PETG** (Polyethylene Terephthalate Glycol) or **ABS/ASA**. PETG combines high tensile strength with UV resistance and ductility, preventing brittle fractures when operating in rugged environments.
- **Infill:** 35% Gyroid infill for isotropic mechanical dampening.
- **Perimeters:** 4 perimeters (wall thickness 1.6mm) to ensure solid thread bite for servo mounting screws.
- **Layer Resolution:** 0.20mm for clean servo spline fitment.

---

## 6. Firmware Architecture & Locomotion Kinematics

### 6.1 Dual-Core FreeRTOS Partitioning
To prevent timing jitter during critical servo PWM generation, the firmware leverages the ESP32's dual-core architecture:
- **Core 0 (Networking & Telemetry):**
  - Manages Wi-Fi SoftAP (`VETRAX_Scout_AP`).
  - Serves responsive HTML/JS UI on port 80.
  - Decodes NMEA sentences from u-blox GPS module via `TinyGPSPlus` on UART2.
  - Answers asynchronous `/gps` JSON requests every 1500ms.
- **Core 1 (Locomotion Engine):**
  - Dedicated real-time hardware LEDC timer execution.
  - Calculates dynamic inverse joint transformations.
  - Executes 4-phase diagonal trot gaits with zero network-induced lag.

### 6.2 4-Phase Diagonal Trot Locomotion
Static stability in quadrupeds requires the Center of Mass (CoM) to remain inside the support polygon formed by grounded feet. VETRAX uses a 4-phase diagonal crawl-trot:
- **Phase 1 (Pair A Lift & Swing):** Diagonal legs RF (`Ch 0, 5`) and LR (`Ch 3, 7`) lift knees to 25° and sweep hips forward to 115°. Stance pair LF & RR remain grounded.
- **Phase 2 (Pair A Touchdown):** Knees lower to 50°, planting feet securely.
- **Phase 3 (Pair B Lift & Swing):** Diagonal legs LF (`Ch 2, 6`) and RR (`Ch 1, 4`) lift knees and swing hips forward.
- **Phase 4 (Pair B Touchdown & Stance Propulsion):** All grounded feet push backwards to neutral 90°, driving the robot chassis forward by approximately 3.5 cm per step cycle.

### 6.3 Watchdog Safety Interlock
Field robotics requires automated failsafes against signal loss. If the user interface drops connection or stops transmitting keepalive heartbeat packets for longer than **3000 ms**, the onboard software watchdog automatically overrides `currentCommand` to `STAND`, locking all servos into the stable 4-point stance and terminating current draw.

---

## 7. Optical Surveillance & Night Tracking Subsystem

The auxiliary **AI-Thinker ESP32-CAM** unit operates in tandem with the quadruped:
- **High-Resolution Streaming:** Streams real-time MJPEG video (VGA 640x480 @ 20 FPS or SVGA 800x600 @ 15 FPS) over HTTP port 80.
- **Low-Light / Night Wildlife Operation:** In darkness or dense canopy shadows, the onboard high-output Flash LED (GPIO 4) can be remotely pulsed via `/flash` to verify elephant presence, herd size, or calf vulnerability.
- **Snapshot Archival:** Field personnel can trigger instantaneous snapshot captures (`/capture`) to store timestamped photographic evidence of elephant incursions for wildlife department records.

---

## 8. Telemetry & GIS Mapping Interface

The built-in web portal automatically parses GPS NMEA sentences ($GPGGA and $GPRMC) into structured JSON:
```json
{
  "fix": true,
  "lat": 11.016844,
  "lng": 76.955832,
  "alt": 412.5,
  "speed": 0.8,
  "satellites": 9,
  "hdop": 1.2
}
```
The client dashboard automatically generates a dynamic hyperlink:
`https://www.google.com/maps/place/<LAT>,<LNG>`  
allowing rangers to click directly into satellite maps, overlay forestry boundary layers, and dispatch patrol units accurately.

---

## 9. Bill of Materials (BOM) & Cost Analysis

| Item Description | Quantity | Est. Unit Cost (USD) | Est. Total Cost (USD) |
|---|---|---|---|
| ESP32 NodeMCU-32S Microcontroller | 1 | $4.50 | $4.50 |
| AI-Thinker ESP32-CAM + OV2640 Module | 1 | $6.50 | $6.50 |
| u-blox NEO-6M GPS Breakout + Ceramic Antenna | 1 | $5.00 | $5.00 |
| TowerPro MG90S Metal-Gear Servos | 8 | $2.80 | $22.40 |
| 5V 5A UBEC DC-DC Step Down Converter | 1 | $4.00 | $4.00 |
| 2S 7.4V 2200mAh LiPo Battery Pack | 1 | $14.00 | $14.00 |
| 3D Printing Filament (PETG - approx. 350g) | 1 | $7.00 | $7.00 |
| M2/M3 Screws, Stand-offs, Servo Horns | 1 Set | $3.50 | $3.50 |
| **TOTAL ESTIMATED SYSTEM COST** | — | — | **$66.90** |

*Note: The sub-$70 total cost allows forestry divisions to deploy multi-agent scout swarms across wide wildlife corridors at a fraction of the cost of a single commercial FLIR drone.*

---

## 10. Verification, Experimental Testing & Performance

| Test Parameter | Observed Performance | Metric Evaluation |
|---|---|---|
| **Max Payload Capacity** | 380 grams | Readily supports ESP32, Camera, GPS & 2S Battery |
| **Walking Speed (Forward)** | 4.2 cm / second | Optimal for silent stalking without acoustic scaring |
| **Battery Operational Life** | ~85 minutes continuous walking; ~4.5 hours idle stand/watch | Tested on 2S 2200mAh pack |
| **GPS Fix Acquisition (Cold Boot)** | 38 seconds outdoors | 7–11 satellites visible under canopy |
| **Wi-Fi Range (Dense Brush)** | 45 – 65 meters Line-of-Sight | Ample safety standoff distance for wildlife personnel |
| **Brownout Reboots** | 0 occurrences in 2 hours stress testing | Verified with brownout register bypass & staggered startup |

---

## 11. Future Scope & Roadmaps

1. **LoRaWAN & Satellite Telemetry:** Integrating an SX1262 LoRa module (868/915 MHz) to relay GPS coordinates up to 12 km through dense hills to central basecamp stations without requiring Wi-Fi.
2. **On-Device Edge AI Elephant Detection:** Deploying TensorFlow Lite for Microcontrollers (TFLite Micro) or Edge Impulse on an ESP32-S3 with dual-camera stereo depth to automatically recognize elephant profiles, count individuals, and issue automated SMS alerts.
3. **Thermal Infrared Imaging:** Mounting an MLX90640 32x24 thermal array to detect warm elephant body heat through thick brush even in pitch-black night conditions.
4. **Energy Harvesting:** Integrating flexible thin-film solar panels on `Top-Cover-Enclosed` to extend deployment times indefinitely in sunlit clearings.

---

## 12. Conclusion

The **VETRAX** autonomous quadruped robot provides an innovative, cost-effective, and humane technological solution to the pressing global issue of Human-Elephant Conflict. By fusing biomimetic 8-DOF quadrupedal locomotion, robust satellite telemetry, optical reconnaissance, and off-grid mobile web interfaces, VETRAX gives conservationists and forest rangers the vital tools needed to protect both endangered wildlife and rural communities safely.
