# 🖨️ VETRAX 3D Printing & Mechanical Assembly Guide

This directory contains all printable **STL 3D CAD mesh files** for constructing the **VETRAX 8-DOF Elephant Monitoring & Wildlife Scout Quadruped Robot**.

---

## 📂 Directory Structure

```text
stl/
├── quadruped_chassis/               # Primary 11-part field chassis & legs
│   ├── Internal-Frame-v121.stl      # Main chassis mounting core (ESP32, GPS, Battery)
│   ├── Top-Cover-Enclosed-v117.stl  # Protective weather & vegetation upper canopy
│   ├── Bottom-Cover-v121.stl        # Undercarriage shield
│   ├── R1-v117.stl                  # Right Front Hip Link (Servo Ch 0 - GPIO 4)
│   ├── R2-v117.stl                  # Right Rear Hip Link (Servo Ch 1 - GPIO 5)
│   ├── L1-v117.stl                  # Left Front Hip Link (Servo Ch 2 - GPIO 13)
│   ├── L2-v117.stl                  # Left Rear Hip Link (Servo Ch 3 - GPIO 14)
│   ├── R4-v117.stl                  # Right Rear Foot/Knee Link (Servo Ch 4 - GPIO 25)
│   ├── R3-v117.stl                  # Right Front Foot/Knee Link (Servo Ch 5 - GPIO 26)
│   ├── L3-v117.stl                  # Left Front Foot/Knee Link (Servo Ch 6 - GPIO 27)
│   └── L4-v117.stl                  # Left Rear Foot/Knee Link (Servo Ch 7 - GPIO 33)
└── spider_chassis/                  # Modular spider/octopod alternative chassis parts
    ├── spid_part_1_obj1.stl         # Main chassis plate upper
    ├── spid_part_5_obj5.stl         # Main chassis plate lower
    ├── spid_part_9_obj9.stl         # Femur joint link
    ├── spid_part_13_obj13.stl       # Tibia joint link
    ├── spid_part_17_obj17.stl       # Coxa/hip servo bracket
    └── ...                          # Auxiliary joint fasteners & covers
```

---

## ⚙️ Recommended 3D Slicer Settings

| Parameter | Recommended Setting | Rationale |
|---|---|---|
| **Material** | **PETG** or **ABS/ASA** *(PLA for indoor testing)* | PETG provides high UV resistance, weather resistance, and flexibility to absorb outdoor trail shocks. |
| **Layer Height** | `0.20 mm` | Balances print speed with fine mechanical tolerances for servo spline fits. |
| **Infill Density** | `30% - 40%` | Ensures mechanical resilience during dynamic diagonal trot gaits and payload carry. |
| **Infill Pattern** | `Gyroid` or `Grid` | Isotropic strength across all Cartesian axes. |
| **Perimeters / Walls** | `4 Walls` | Guarantees strength where M2/M3 self-tapping servo screws bite into the plastic. |
| **Top / Bottom Layers** | `4 Top / 4 Bottom` | Ensures sealed enclosure protection against dust and moisture. |
| **Supports** | **Tree Supports** (Auto, 45° threshold) | Easy detachment around servo mounting recesses and axle pivots. |
| **Print Bed Adhesion** | Brim (4–5 mm) | Prevents corner warping on long leg segments. |

---

## 🔩 Hardware Bill of Materials (Mechanical)

- **8x Micro Servos**: SG90 or MG90S (Metal Gear recommended for outdoor field durability).
- **M2 Screws & Servo Horn Screws**: For securing servo horns to `R1–R4` and `L1–L4` linkages.
- **M3 x 8mm / M3 x 12mm Button Head Screws**: For joining `Internal-Frame-v121` to top/bottom covers.
- **Rubber / Silicone Foot Pads**: Optional traction pads attached to foot tips (`R3, R4, L3, L4`) to enhance traction on forest humus and gravel.

---

## 🛠️ Assembly & Calibration Sequence

1. **Servo Centering (90° Neutral)**:
   - Before mounting any servo horns into the 3D printed links, flash the firmware and run the `STAND` routine (which sets all servos to their neutral 90° reference).
2. **Hip Link Installation (`R1, R2, L1, L2`)**:
   - Fasten the hip links perpendicularly (90° to the chassis centerline) so they have an equal ±45° sweep range forward and backward.
3. **Knee / Foot Link Installation (`R3, R4, L3, L4`)**:
   - Secure the knee linkages so the robot stands upright with feet planted stably at standard 45° stance angle.
4. **Wiring & Routing**:
   - Route servo lead wires through the central wire channels inside `Internal-Frame-v121.stl`.
   - Mount the ESP32 / ESP32-CAM and Neo-6M GPS antenna facing upwards on the designated top-plate standoff mounts.
