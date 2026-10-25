# 🦿 VETRAX Quadruped Kinematics & Locomotion Gaits

This document describes the mathematical kinematics, gait generation, and stability analysis for the **VETRAX 8-DOF Elephant Monitoring Quadruped Robot**.

---

## 📐 Kinematic Configuration

The robot operates with 4 limbs arranged symmetrically around the central chassis frame. Each limb possesses 2 Degrees of Freedom (2-DOF):
1. **Coxa / Hip Joint (Yaw/Pitch)**: Sweeps the leg forward and backward along the longitudinal axis of travel.
2. **Knee / Foot Joint (Pitch)**: Lifts and lowers the foot vertically relative to the ground plane.

```text
         [FRONT OF ROBOT]
     LF (Ch 2, 6)   RF (Ch 0, 5)
          \             /
           \           /
            +---------+
            | CHASSIS |
            +---------+
           /           \
          /             \
     LR (Ch 3, 7)   RR (Ch 1, 4)
          [REAR OF ROBOT]
```

---

## 🔄 Symmetry & Mirroring Mapping

Because the servos on the left and right sides of the chassis face opposite directions, an identical angular rotation would cause opposite leg swings. The firmware resolves this using the `baseZero` offset lookup:

$$\theta_{\text{final}} = \begin{cases} 90^\circ - (\theta_{\text{input}} - 90^\circ) & \text{if } \text{baseZero}[ch] = 90^\circ \\ \theta_{\text{input}} & \text{if } \text{baseZero}[ch] = 0^\circ \end{cases}$$

This ensures that a single high-level command (e.g., `swingForward = +25°`) translates into symmetric forward motion on both sides.

---

## 🏃 4-Phase Diagonal Trot Gait

To ensure dynamic stability on irregular forest trails without tipping, VETRAX uses a **diagonal trot crawl gait**. The legs are grouped into two diagonal pairs:
- **Diagonal Pair A:** Right Front (`RF`) + Left Rear (`LR`)
- **Diagonal Pair B:** Left Front (`LF`) + Right Rear (`RR`)

### Gait Phase Cycle:
1. **Phase 1 (Pair A Swing):**
   - Knees of Pair A (`Ch 5, 7`) lift the feet off the ground (`25°`).
   - Hips of Pair A (`Ch 0, 3`) swing forward (`115°`).
   - Pair B remains firmly grounded, providing a stable 2-point stance tripod base.
2. **Phase 2 (Pair A Touchdown):**
   - Knees of Pair A lower to ground contact (`50°`).
   - All four feet are momentarily grounded for maximum stability.
3. **Phase 3 (Pair B Swing):**
   - Knees of Pair B (`Ch 6, 4`) lift the feet off the ground (`25°`).
   - Hips of Pair B (`Ch 2, 1`) swing forward (`65°` mirrored).
   - Pair A provides the supporting stance.
4. **Phase 4 (Pair B Touchdown & Propulsion):**
   - Knees of Pair B lower to ground contact (`50°`).
   - All hip joints execute stance retraction towards neutral (`90°`), propelling the chassis forward smoothly.

---

## 🛡️ Dynamic Watchdog & Failsafe

In wildlife monitoring scenarios, continuous radio contact can occasionally be interrupted by dense tree canopies or terrain obstructions. 
- If no HTTP keepalive or movement command packet is received within **3000 ms**, the onboard watchdog timer triggers.
- The robot immediately transitions to `STAND` (neutral stability pose), cutting dynamic servo draw and preventing the robot from blindly walking into obstacles or elephant trails.
