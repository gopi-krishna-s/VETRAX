/**
 * =========================================================================================
 * VETRAX: Wildlife & Elephant Monitoring Scout Robot — Autonomous Tracker
 * =========================================================================================
 * Hardware Platform: ESP32 NodeMCU-32S / ESP32-WROOM-32
 * Actuation        : 8x Direct GPIO Micro Servos (MG90S / SG90)
 * Telemetry & Nav  : u-blox Neo-6M GPS Module via UART2 (GPIO 16 RX / 17 TX)
 * Networking       : SoftAP WebServer (192.168.4.1) + Mobile Control Dashboard
 * 
 * Pinout Channel Configuration:
 *   Ch 0: Right Front Hip   (GPIO 4)  | Ch 4: Right Rear Knee   (GPIO 25)
 *   Ch 1: Right Rear Hip    (GPIO 5)  | Ch 5: Right Front Knee  (GPIO 26)
 *   Ch 2: Left Front Hip    (GPIO 13) | Ch 6: Left Front Knee   (GPIO 27)
 *   Ch 3: Left Rear Hip     (GPIO 14) | Ch 7: Left Rear Knee    (GPIO 33)
 * =========================================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// ESP32 brownout detector control register
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- SERVO CONFIGURATION ---
// 8 Direct GPIO pins matching 3D printed mechanical linkages
const int servoPins[8] = {4, 5, 13, 14, 25, 26, 27, 33};
Servo legServos[8];

// Base zero offset mapping: accounts for left-right mechanical servo mirroring
// 90 = mirrored (90 - angle), 0 = direct (0 + angle)
const int baseZero[8] = {90, 0, 0, 90, 90, 0, 90, 0};
int currentAngles[8] = {90, 90, 90, 90, 90, 90, 90, 90};

// --- GPS CONFIGURATION ---
TinyGPSPlus gps;
HardwareSerial SerialGPS(2); // UART2
const int RXPin = 16;        // ESP32 RX2 connected to GPS TX
const int TXPin = 17;        // ESP32 TX2 connected to GPS RX
const uint32_t GPSBaud = 9600;

// --- WI-FI & WEB SERVER CONFIGURATION ---
const char* ssid = "VETRAX_Scout_AP";      // Broadcasted Access Point
const char* password = "elephant123";      // WPA2 Passphrase (min 8 chars)
WebServer server(80);

// Locomotion state and step delay
String currentCommand = "STOP";
int stepDelay = 180; // ms per gait phase (adjust for terrain speed)
unsigned long lastCommandTime = 0;
const unsigned long WATCHDOG_TIMEOUT_MS = 3000; // Auto-stop if command times out

// --- RESPONSIVE WEB DASHBOARD ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>VETRAX Elephant Monitor</title>
  <style>
    :root {
      --bg: #121824;
      --card: #1c2436;
      --primary: #3b82f6;
      --primary-hover: #2563eb;
      --danger: #ef4444;
      --text: #f3f4f6;
      --accent: #10b981;
      --gps-color: #f59e0b;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background-color: var(--bg); color: var(--text); padding: 16px; display: flex; flex-direction: column; align-items: center; min-height: 100vh; }
    header { text-align: center; margin-bottom: 16px; }
    h1 { font-size: 1.5rem; letter-spacing: 1px; color: #60a5fa; }
    .subtitle { font-size: 0.85rem; color: #9ca3af; margin-top: 4px; }
    .card { background: var(--card); border: 1px solid rgba(255,255,255,0.08); border-radius: 14px; padding: 16px; width: 100%; max-width: 480px; margin-bottom: 16px; box-shadow: 0 4px 12px rgba(0,0,0,0.3); }
    .gps-title { font-size: 0.85rem; text-transform: uppercase; color: var(--gps-color); font-weight: 700; margin-bottom: 8px; display: flex; justify-content: space-between; }
    .gps-coords { font-size: 1.05rem; font-family: monospace; line-height: 1.5; color: #f9fafb; }
    .map-btn { display: inline-block; margin-top: 10px; padding: 8px 14px; background: rgba(245,158,11,0.15); border: 1px solid var(--gps-color); color: var(--gps-color); text-decoration: none; border-radius: 8px; font-size: 0.85rem; font-weight: 600; }
    .dpad-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; justify-items: center; align-items: center; max-width: 300px; margin: 0 auto; }
    .btn { width: 85px; height: 85px; border-radius: 14px; border: none; font-size: 1.1rem; font-weight: 700; cursor: pointer; color: white; background: var(--primary); transition: 0.1s; display: flex; flex-direction: column; justify-content: center; align-items: center; gap: 4px; }
    .btn:active { background: var(--primary-hover); transform: scale(0.95); }
    .btn-stop { background: var(--danger); width: 85px; height: 85px; }
    .btn-stop:active { background: #b91c1c; }
    .aux-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; margin-top: 14px; }
    .aux-btn { padding: 10px; background: rgba(255,255,255,0.07); border: 1px solid rgba(255,255,255,0.15); color: #e5e7eb; border-radius: 8px; font-size: 0.8rem; font-weight: 600; cursor: pointer; }
    .status-bar { font-size: 0.8rem; color: #9ca3af; text-align: center; margin-top: 8px; }
  </style>
</head>
<body>
  <header>
    <h1>🐘 VETRAX SCOUT</h1>
    <div class="subtitle">Autonomous Wildlife Reconnaissance Unit</div>
  </header>

  <!-- GPS Telemetry Card -->
  <div class="card">
    <div class="gps-title">
      <span>📍 GPS Navigation Telemetry</span>
      <span id="satCount">🛰️ 0 Sats</span>
    </div>
    <div class="gps-coords" id="gpsData">Acquiring GPS fix...</div>
    <div id="mapContainer"></div>
  </div>

  <!-- Locomotion D-Pad Card -->
  <div class="card">
    <div class="dpad-grid">
      <div></div>
      <button class="btn" onclick="sendCmd('FORWARD')"><span>▲</span>FWD</button>
      <div></div>
      <button class="btn" onclick="sendCmd('LEFT')"><span>◄</span>LEFT</button>
      <button class="btn btn-stop" onclick="sendCmd('STOP')"><span>■</span>STOP</button>
      <button class="btn" onclick="sendCmd('RIGHT')"><span>►</span>RIGHT</button>
      <div></div>
      <button class="btn" onclick="sendCmd('BACKWARD')"><span>▼</span>REV</button>
      <div></div>
    </div>
    <div class="aux-grid">
      <button class="aux-btn" onclick="sendCmd('STAND')">🦿 Stand</button>
      <button class="aux-btn" onclick="sendCmd('CROUCH')">🔽 Crouch</button>
      <button class="aux-btn" onclick="sendCmd('SIT')">🛑 Sit</button>
    </div>
    <div class="status-bar" id="statusText">System Ready (Idle: Stand)</div>
  </div>

  <script>
    function sendCmd(cmd) {
      document.getElementById('statusText').innerText = 'Mode: ' + cmd;
      fetch('/action?go=' + cmd).catch(err => console.error(err));
    }

    // Periodically fetch GPS telemetry
    setInterval(function() {
      fetch('/gps').then(r => r.json()).then(data => {
        if (data.fix) {
          document.getElementById('gpsData').innerHTML = 
            'Lat: ' + data.lat.toFixed(6) + '<br>Lng: ' + data.lng.toFixed(6) + 
            '<br>Alt: ' + data.alt.toFixed(1) + ' m | Speed: ' + data.speed.toFixed(1) + ' km/h';
          document.getElementById('satCount').innerText = '🛰️ ' + data.satellites + ' Sats';
          document.getElementById('mapContainer').innerHTML = 
            '<a class="map-btn" href="https://www.google.com/maps/place/' + 
            data.lat + ',' + data.lng + '" target="_blank">🌐 View on Google Maps</a>';
        } else {
          document.getElementById('gpsData').innerText = 'Searching satellites... (' + data.satellites + ' visible)';
          document.getElementById('satCount').innerText = '🛰️ ' + data.satellites + ' Sats';
        }
      }).catch(e => {
        document.getElementById('gpsData').innerText = 'Telemetry offline';
      });
    }, 1500);
  </script>
</body>
</html>
)rawliteral";

// --- SERVO KINEMATICS HELPER FUNCTIONS ---
int calculateAngle(int channel, int targetAngle) {
  if (baseZero[channel] == 90) {
    return constrain(90 - (targetAngle - 90), 0, 180);
  } else {
    return constrain(targetAngle, 0, 180);
  }
}

void moveJoint(int channel, int angle) {
  if (channel >= 0 && channel < 8) {
    int finalAngle = calculateAngle(channel, angle);
    legServos[channel].write(finalAngle);
    currentAngles[channel] = angle;
  }
}

// Smooth transition of all servos to a target pose
void setAllJoints(int hipAngle, int kneeAngle) {
  for (int i = 0; i < 4; i++) moveJoint(i, hipAngle);
  for (int i = 4; i < 8; i++) moveJoint(i, kneeAngle);
}

// --- PRESET ROBOT POSES ---
void robotStand() {
  // Neutral upright stance (Hips 90°, Knees 45° lift)
  setAllJoints(90, 50);
}

void robotCrouch() {
  // Low-profile stealth / surveillance stance
  setAllJoints(90, 80);
}

void robotSit() {
  // Rest posture, fold legs close to body
  setAllJoints(90, 135);
}

// --- 4-PHASE DIAGONAL LOCOMOTION GAITS ---
// Diagonal Pair A: Right Front (RF: Hip Ch0, Knee Ch5) & Left Rear (LR: Hip Ch3, Knee Ch7)
// Diagonal Pair B: Left Front (LF: Hip Ch2, Knee Ch6) & Right Rear (RR: Hip Ch1, Knee Ch4)

void walkForwardStep() {
  // Phase 1: Lift Diagonal A (RF & LR), swing forward
  moveJoint(5, 25); moveJoint(7, 25); // Lift knees
  delay(stepDelay / 2);
  moveJoint(0, 115); moveJoint(3, 115); // Swing hips forward
  delay(stepDelay / 2);

  // Phase 2: Ground Diagonal A
  moveJoint(5, 50); moveJoint(7, 50);
  delay(stepDelay / 2);

  // Phase 3: Lift Diagonal B (LF & RR), swing forward
  moveJoint(6, 25); moveJoint(4, 25); // Lift knees
  delay(stepDelay / 2);
  moveJoint(2, 65); moveJoint(1, 65);  // Swing hips forward
  delay(stepDelay / 2);

  // Phase 4: Ground Diagonal B
  moveJoint(6, 50); moveJoint(4, 50);
  delay(stepDelay / 2);

  // Propel stance legs back to neutral
  moveJoint(0, 90); moveJoint(3, 90);
  moveJoint(2, 90); moveJoint(1, 90);
}

void walkBackwardStep() {
  // Phase 1: Lift Diagonal A, swing backward
  moveJoint(5, 25); moveJoint(7, 25);
  delay(stepDelay / 2);
  moveJoint(0, 65); moveJoint(3, 65);
  delay(stepDelay / 2);
  moveJoint(5, 50); moveJoint(7, 50);
  delay(stepDelay / 2);

  // Phase 2: Lift Diagonal B, swing backward
  moveJoint(6, 25); moveJoint(4, 25);
  delay(stepDelay / 2);
  moveJoint(2, 115); moveJoint(1, 115);
  delay(stepDelay / 2);
  moveJoint(6, 50); moveJoint(4, 50);
  delay(stepDelay / 2);

  // Return to neutral
  moveJoint(0, 90); moveJoint(3, 90);
  moveJoint(2, 90); moveJoint(1, 90);
}

void turnLeftStep() {
  // Turn body left by moving opposite diagonal hips
  moveJoint(5, 25); moveJoint(4, 25);
  delay(stepDelay / 2);
  moveJoint(0, 115); moveJoint(1, 115);
  delay(stepDelay / 2);
  moveJoint(5, 50); moveJoint(4, 50);
  delay(stepDelay / 2);

  moveJoint(6, 25); moveJoint(7, 25);
  delay(stepDelay / 2);
  moveJoint(2, 65); moveJoint(3, 65);
  delay(stepDelay / 2);
  moveJoint(6, 50); moveJoint(7, 50);
  delay(stepDelay / 2);

  robotStand();
}

void turnRightStep() {
  // Turn body right by moving opposite diagonal hips
  moveJoint(6, 25); moveJoint(7, 25);
  delay(stepDelay / 2);
  moveJoint(2, 115); moveJoint(3, 115);
  delay(stepDelay / 2);
  moveJoint(6, 50); moveJoint(7, 50);
  delay(stepDelay / 2);

  moveJoint(5, 25); moveJoint(4, 25);
  delay(stepDelay / 2);
  moveJoint(0, 65); moveJoint(1, 65);
  delay(stepDelay / 2);
  moveJoint(5, 50); moveJoint(4, 50);
  delay(stepDelay / 2);

  robotStand();
}

// --- SETUP ROUTINE ---
void setup() {
  // 1. Disable ESP32 brownout detector to prevent reboot on servo current spike
  #ifdef RTC_CNTL_BROWN_OUT_REG
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  #endif

  Serial.begin(115200);
  delay(200);
  Serial.println("\n=======================================================");
  Serial.println(" 🐘 VETRAX: Wildlife & Elephant Monitoring Scout Robot ");
  Serial.println("=======================================================");

  // 2. Initialize Neo-6M GPS on UART2
  SerialGPS.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  Serial.printf(" ✅ GPS Initialized on RX=%d, TX=%d @ %d baud\n", RXPin, TXPin, GPSBaud);

  // 3. Allocate PWM timers and initialize 8 Direct Servos
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  for (int ch = 0; ch < 8; ch++) {
    legServos[ch].setPeriodHertz(50); // Standard 50Hz (20ms) servo frequency
    legServos[ch].attach(servoPins[ch], 500, 2400); // Standard pulse width 500us - 2400us
    int neutral = calculateAngle(ch, 90);
    legServos[ch].write(neutral);
    Serial.printf(" ✅ Servo Ch%d on GPIO %d attached (Neutral: %d°)\n", ch, servoPins[ch], neutral);
    delay(40); // Staggered initialization avoids inrush current
  }

  // Settle in stand pose
  robotStand();

  // 4. Setup Wi-Fi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print(" 📡 Access Point Started: ");
  Serial.println(ssid);
  Serial.print(" 🌐 Web Dashboard IP    : ");
  Serial.println(WiFi.softAPIP());

  // 5. Setup Web Server Endpoints
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/action", HTTP_GET, []() {
    if (server.hasArg("go")) {
      currentCommand = server.arg("go");
      lastCommandTime = millis();
      Serial.println(" 🎮 Command Received: " + currentCommand);
    }
    server.send(200, "text/plain", "OK");
  });

  // Telemetry endpoint returning JSON
  server.on("/gps", HTTP_GET, []() {
    String json = "{";
    json += "\"fix\":" + String(gps.location.isValid() ? "true" : "false") + ",";
    json += "\"lat\":" + String(gps.location.lat(), 6) + ",";
    json += "\"lng\":" + String(gps.location.lng(), 6) + ",";
    json += "\"alt\":" + String(gps.altitude.meters(), 1) + ",";
    json += "\"speed\":" + String(gps.speed.kmph(), 1) + ",";
    json += "\"satellites\":" + String(gps.satellites.value()) + ",";
    json += "\"hdop\":" + String(gps.hdop.hdop(), 1);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println(" 🚀 Web Server listening on port 80");
}

// --- MAIN LOOP ---
void loop() {
  // 1. Service HTTP Web Clients
  server.handleClient();

  // 2. Continual GPS stream reading
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  // 3. Watchdog check: Auto-stop if no command received for timeout duration
  if (currentCommand != "STOP" && currentCommand != "STAND" && currentCommand != "SIT" && currentCommand != "CROUCH") {
    if (millis() - lastCommandTime > WATCHDOG_TIMEOUT_MS) {
      Serial.println(" ⚠️ Watchdog: Motion command timed out. Returning to STAND.");
      currentCommand = "STOP";
      robotStand();
    }
  }

  // 4. Locomotion Execution
  if (currentCommand == "STOP" || currentCommand == "STAND") {
    robotStand();
  } else if (currentCommand == "FORWARD") {
    walkForwardStep();
  } else if (currentCommand == "BACKWARD") {
    walkBackwardStep();
  } else if (currentCommand == "LEFT") {
    turnLeftStep();
  } else if (currentCommand == "RIGHT") {
    turnRightStep();
  } else if (currentCommand == "CROUCH") {
    robotCrouch();
  } else if (currentCommand == "SIT") {
    robotSit();
  }
}
