<div align="center">
  <img src="t-photos/prometehuslogo.jpg" alt="Prometheus Team" width="600">

# 🔥 Prometheus Team · WRO 2026 Future Engineers

**Universidad Politécnica Territorial José Félix Ribas (UPTJFR) · Barinas, Venezuela**

![WRO](https://img.shields.io/badge/WRO_2026-Future_Engineers-0057B8?style=for-the-badge)
![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino_IDE-00878F?style=for-the-badge&logo=arduino&logoColor=white)
![Android](https://img.shields.io/badge/Android_Studio-3DDC84?style=for-the-badge&logo=androidstudio&logoColor=white)
![OpenCV](https://img.shields.io/badge/OpenCV-5C3EE8?style=for-the-badge&logo=opencv&logoColor=white)
![Onshape](https://img.shields.io/badge/Onshape-CAD-0D3C61?style=for-the-badge)

</div>

---

## 🥈 2025 National Runners-Up · Aiming for First Place in 2026

Welcome to the official repository of **Prometheus Team**, a Venezuelan team competing in the **World Robot Olympiad (WRO) Future Engineers category** on behalf of the **Universidad Politécnica Territorial José Félix Ribas (UPTJFR)**. Here you will find everything you need to understand and replicate our autonomous vehicle: the ESP32 code, the computer vision app, the 3D parts, the electrical schematics, and a record of how we solved each problem along the way.

In 2025 we finished as **national runners-up**. For 2026 we redesigned almost the entire vehicle based on what failed in that final: a new chassis with a 1:1 ratio, Ackermann steering, a single ESP32, ToF sensors, and a BNO055 IMU.

<div align="center">

| 📏 Dimensions | ⚖️ Weight | 🛞 Turning radius | 🔋 Battery life | ⏱️ Open challenge lap | 🚧 Obstacle challenge lap |
| :---: | :---: | :---: | :---: | :---: | :---: |
| 179 × 143 mm | 1092 g (with phone) | 176 mm | ≈ 1 h 54 min | 25 s | 80 s |

<sub>Average lap times measured on our 3 × 3 m test track.</sub>

</div>

---

## 📑 Table of Contents

<table>
<tr><td valign="top" width="50%">

**👥 Team and Project**

- [1. 👥 Prometheus Team](#1--prometheus-team)
  - [1.1. 👤 Samuel Pérez](#11--samuel-pérez)
  - [1.2. 👤 Trino Carrizales](#12--trino-carrizales)
  - [1.3. 👤 Leandro Padrón](#13--leandro-padrón)
  - [1.4. 🎓 Eng. Dayana Pérez](#14--eng-dayana-pérez)
  - [1.5. 🏆 From 2025 Runners-Up to First Place in 2026](#15--from-2025-runners-up-to-first-place-in-2026)
  - [1.6. 📸 Our Run at the National Competition](#16--our-run-at-the-national-competition)
- [2. 📂 Repository Structure](#2--repository-structure)
- [3. 🤖 Our Vehicle](#3--our-vehicle)
  - [3.1. 📋 General Specifications](#31--general-specifications)
  - [3.2. 🧩 System Architecture](#32--system-architecture)
  - [3.3. 🔄 360° Technical Inspection Gallery](#33--360-technical-inspection-gallery)

**🔌 Electronics and Power**

- [4. 🧠 Control, Sensing, and Interface Components](#4--control-sensing-and-interface-components)
- [5. 🔌 Power and Actuation Components](#5--power-and-actuation-components)
- [6. ⚡ Power and Wiring Schematics](#6--power-and-wiring-schematics)
  - [6.1. 🧾 Version 4 (current)](#61--version-4-current)
  - [6.2. 🔋 Power, Control, and Power Management Architecture](#62--power-control-and-power-management-architecture)
- [7. 📡 Sensor and Data Acquisition Schematics](#7--sensor-and-data-acquisition-schematics)
  - [7.1. 🧾 Version 4 (current)](#71--version-4-current)
  - [7.2. 🔧 Sensor and Hardware Optimization](#72--sensor-and-hardware-optimization)
  - [7.3. 📋 Summary of Hardware Changes](#73--summary-of-hardware-changes)
- [8. 🔋 Power Management and Battery Life](#8--power-management-and-battery-life)
  - [8.1. 📊 Total Power Consumption](#81--total-power-consumption)
  - [8.2. ⏳ Battery Life Estimate](#82--battery-life-estimate)
  - [8.3. 🔁 DC-DC Converter Efficiency and Actual Battery Life](#83--dc-dc-converter-efficiency-and-actual-battery-life)
  - [8.4. ✅ Power Source Justification](#84--power-source-justification)

**⚙️ Mechanical Design**

- [9. 🧱 Structural Design, Drivetrain, and Additive Manufacturing](#9--structural-design-drivetrain-and-additive-manufacturing)
  - [9.1. 📝 Redesign Approach and Concept Prototyping](#91--redesign-approach-and-concept-prototyping)
  - [9.2. 🚙 Moving to Professional CAD and the Monochassis Design](#92--moving-to-professional-cad-and-the-monochassis-design)
  - [9.3. 📏 Tolerance Validation and Virtual Assembly](#93--tolerance-validation-and-virtual-assembly)
  - [9.4. 🛞 Ackermann Steering System](#94--ackermann-steering-system)
  - [9.5. 🦈 Second Level, Standoffs, and the Shark Fin Vision System](#95--second-level-standoffs-and-the-shark-fin-vision-system)
  - [9.6. 🧩 Full Integration: CAD Master Assembly](#96--full-integration-cad-master-assembly)
  - [9.7. 🏭 Additive Manufacturing](#97--additive-manufacturing)
  - [9.8. 📐 Final Dimensions and Summary of Structural Improvements](#98--final-dimensions-and-summary-of-structural-improvements)

</td><td valign="top" width="50%">

**🧮 Calculations, Evolution, and Assembly**

- [10. 🧮 Mass Analysis and Weight Distribution](#10--mass-analysis-and-weight-distribution)
  - [10.1. 📦 Vehicle Mass: 2025 vs. 2026](#101--vehicle-mass-2025-vs-2026)
  - [10.2. 📊 Static Mass Distribution (Front Axle / Rear Axle)](#102--static-mass-distribution-front-axle--rear-axle)
- [11. 🚀 Speed, Torque, and Traction](#11--speed-torque-and-traction)
  - [11.1. 💨 Vehicle Linear Speed](#111--vehicle-linear-speed)
  - [11.2. 🔩 Required Torque vs. Available Torque](#112--required-torque-vs-available-torque)
  - [11.3. 🔒 Torque Safety Margin](#113--torque-safety-margin)
  - [11.4. ⏳ Theoretical vs. Actual Lap Time](#114--theoretical-vs-actual-lap-time)
  - [11.5. 🛞 Friction Coefficient and Grip](#115--friction-coefficient-and-grip)
  - [11.6. 🔗 Drivetrain Alternatives Considered: Gears vs. Timing Belt](#116--drivetrain-alternatives-considered-gears-vs-timing-belt)
- [12. 📓 Engineering Log: Chassis Evolution (v0 → v2)](#12--engineering-log-chassis-evolution-v0--v2)
  - [12.1. 🟤 Iteration v0: Cardboard Prototype](#121--iteration-v0-cardboard-prototype)
  - [12.2. 🧪 Iteration v1: First 3D-Printed Chassis](#122--iteration-v1-first-3d-printed-chassis)
  - [12.3. ✅ Iteration v2: Monochassis (Current Version, 2026)](#123--iteration-v2-monochassis-current-version-2026)
- [13. 🔧 Step-by-Step Assembly Guide](#13--step-by-step-assembly-guide)
  - [13.1. 🔹 Step 1: 3D Printing](#131--step-1-3d-printing)
  - [13.2. 🔹 Step 2: Motor and Pinion on the Monochassis](#132--step-2-motor-and-pinion-on-the-monochassis)
  - [13.3. 🔹 Step 3: Rear Axle, Spur Gear, and Wheel Adapters](#133--step-3-rear-axle-spur-gear-and-wheel-adapters)
  - [13.4. 🔹 Step 4: Ackermann Steering](#134--step-4-ackermann-steering)
  - [13.5. 🔹 Step 5: ToF Sensors](#135--step-5-tof-sensors)
  - [13.6. 🔹 Step 6: Standoffs](#136--step-6-standoffs)
  - [13.7. 🔹 Step 7: Shark Fin System (Phone Mount)](#137--step-7-shark-fin-system-phone-mount)
  - [13.8. 🔹 Step 8: Final Integration](#138--step-8-final-integration)

**💻 Software and Testing**

- [14. 💻 Software and Control](#14--software-and-control)
  - [14.1. 🧭 Software Continuity and BNO055 Integration](#141--software-continuity-and-bno055-integration)
  - [14.2. 📁 Firmware Files and Libraries](#142--firmware-files-and-libraries)
  - [14.3. 🔄 Shared Navigation Logic](#143--shared-navigation-logic)
  - [14.4. 🏁 Open Lap Mode (Track Without Obstacles)](#144--open-lap-mode-track-without-obstacles)
  - [14.5. 🚧 Obstacle Mode: Lane System](#145--obstacle-mode-lane-system)
  - [14.6. 🧠 Control Architecture and State Machine](#146--control-architecture-and-state-machine)
- [15. 📱 Telemetry and Computer Vision: Prometheus V5.0 App](#15--telemetry-and-computer-vision-prometheus-v50-app)
  - [15.1. 📺 V5.0 Interface and Built-In Serial Monitor](#151--v50-interface-and-built-in-serial-monitor)
  - [15.2. 🎯 On-Track Calibration and Dynamic Menu](#152--on-track-calibration-and-dynamic-menu)
  - [15.3. ✨ Anti-Glare Filter and Shape Cleanup](#153--anti-glare-filter-and-shape-cleanup)
  - [15.4. 🟣 Parking Mode (Magenta Zone)](#154--parking-mode-magenta-zone)
  - [15.5. ⏳ Time-Based Stabilization Filter](#155--time-based-stabilization-filter)
  - [15.6. 👀 Dual Detection and Case-Based Architecture](#156--dual-detection-and-case-based-architecture)
  - [15.7. 📐 Mathematical and Spatial Model](#157--mathematical-and-spatial-model)
  - [15.8. 📋 Complete Command Matrix (CommandManager)](#158--complete-command-matrix-commandmanager)
  - [15.9. 🔌 USB-Serial Protocol and Fault Tolerance](#159--usb-serial-protocol-and-fault-tolerance)
- [16. 🎬 Track Tests and Videos](#16--track-tests-and-videos)

**🚀 Improvements and Appendices**

- [17. 🚀 Upcoming Improvements](#17--upcoming-improvements)
  - [17.1. 📡 New Sensor Layout](#171--new-sensor-layout)
  - [17.2. ✅ What We Expect to Gain from Each Change](#172--what-we-expect-to-gain-from-each-change)
  - [17.3. 🔧 What We Need to Adjust to Integrate Them](#173--what-we-need-to-adjust-to-integrate-them)
- [18. 🌐 Use of GitHub and Artificial Intelligence](#18--use-of-github-and-artificial-intelligence)
  - [18.1. 🌿 Workflow](#181--workflow)
  - [18.2. 🤖 Use of Artificial Intelligence](#182--use-of-artificial-intelligence)
- [19. 📁 Archive: 2025 Season](#19--archive-2025-season)
  - [19.1. 🚗 2025 Vehicle Overview](#191--2025-vehicle-overview)
  - [19.2. 🔌 Electronics Evolution](#192--electronics-evolution)
  - [19.3. 🔧 Chassis Construction](#193--chassis-construction)
  - [19.4. 📱 Vision App Evolution (V1–V3)](#194--vision-app-evolution-v1v3)
  - [19.5. 🧩 Key Problems and Solutions](#195--key-problems-and-solutions)
  - [19.6. 🎬 Videos and Source Code](#196--videos-and-source-code)

</td></tr>
</table>

---

## 1. 👥 Prometheus Team

Prometheus Team is made up of Electronics Engineering students from the **Universidad Politécnica Territorial "José Félix Ribas"** and their mentor. We already competed together at WRO 2025, where our phone-based obstacle detection system drew attention.

<div align="center">
<table>
<tr>
<td align="center" width="25%"><img src="t-photos/samuel.jpeg" alt="Samuel Pérez" width="170"><br><b>Samuel Pérez</b><br><sub>3D design, CAD, and Android app</sub></td>
<td align="center" width="25%"><img src="t-photos/trino.jpeg" alt="Trino Carrizales" width="170"><br><b>Trino Carrizales</b><br><sub>Firmware, algorithms, and control</sub></td>
<td align="center" width="25%"><img src="t-photos/leandro.jpeg" alt="Leandro Padrón" width="170"><br><b>Leandro Padrón</b><br><sub>Mechanics, assembly, and logistics</sub></td>
<td align="center" width="25%"><img src="t-photos/dayana.jpeg" alt="Eng. Dayana Pérez" width="170"><br><b>Eng. Dayana Pérez</b><br><sub>Team mentor</sub></td>
</tr>
</table>
</div>

### 1.1. 👤 Samuel Pérez
**Mechanical CAD Design, Electronics, and Mobile Development**

* **Education:** Electronics Engineering student at **UPTJFR**.
* **On the team:**
  * **Mechanical design, simulation, and calculations:** parametric modeling in **Onshape**, kinematic simulation, Ackermann steering calculations, motor torque calculations, and gear ratios.
  * **3D manufacturing, assembly, and electronics:** validation of the full assembly in CAD, 3D printing, and physical assembly of the vehicle, along with the design, building, and wiring of the electronic circuit.
  * **Mobile development:** built and debugged the obstacle detection app in **Android Studio** with **OpenCV**.
  * **Achievements:** finalist in the national round of **WRO Venezuela 2025**.

### 1.2. 👤 Trino Carrizales
**Embedded Programming, Algorithms, and Control**

* **Education:**
  * Electronics Engineering student at **UPTJFR**.
  * Systems Engineering student at the **Universidad Nacional Experimental de los Llanos Occidentales Ezequiel Zamora (UNELLEZ)**.
* **On the team:**
  * **Firmware:** programs and optimizes the code for the **ESP32** microcontroller.
  * **Debugging:** finds and fixes logic errors during testing, even in the middle of a competition.
  * **Achievements:** finalist in the national round of **WRO Venezuela 2025**.

### 1.3. 👤 Leandro Padrón
**Logistics, Applied Mechanics, and Physical Troubleshooting**

* **Education:** Electronics Engineering student at **UPTJFR**.
* **On the team:**
  * **Mechanics:** builds the prototypes and fixes the mechanical failures that come up during testing.
  * **Logistics:** coordinates materials, tools, and planning for competition trips.
  * **Trackside work:** diagnoses and repairs failures quickly so the team can keep testing.

### 1.4. 🎓 Eng. Dayana Pérez
**Lead Mentor and Research Advisor**

* **Education:**
  * **Electronics Engineer:** Universidad Nacional Experimental Politécnica Antonio José de Sucre (**UNEXPO**, Barquisimeto campus).
  * **Master's in Higher Education:** Universidad Fermín Toro (**UFT**).
  * **PhD candidate in Information and Communication Technologies:** **UNELLEZ** Barinas.
* **On the team:** methodological guidance, academic supervision of the project, and advice on integrating ICT and electronic systems.

### 1.5. 🏆 From 2025 Runners-Up to First Place in 2026

In 2025 we took **2nd place at the Venezuelan national competition**. We got there after months of design work, sensor debugging, and tuning of the navigation logic, and at the final we had to recalibrate components against the clock and handle surprises on the track against the best teams in the country.

This year we are back with a clear list of what went wrong: we redesigned the chassis, replaced much of the hardware, and improved the control algorithms so the vehicle is faster and more accurate. Our goal is **1st place nationally** and qualification for the international final. The 2025 vehicle is documented in the [2025 season archive](#19--archive-2025-season).

### 1.6. 📸 Our Run at the National Competition

<p align="center">
  <img src="t-photos/foto_equipo.jpeg" alt="Official team photo" height="250" />
  <img src="t-photos/foto_equipo2.jpeg" alt="Team at the competition" height="250" />
  <img src="other/wro2025-1.jpeg" alt="WRO 2025 moment" height="250" />
</p>

<details>
  <summary>🔍 <b>See more moments from WRO 2025</b></summary>
  <br>
  <p align="center">
    <img src="other/wro2025-2.jpeg" alt="WRO 2025, photo 2" height="200" />
    <img src="other/wro2025-3.jpeg" alt="WRO 2025, photo 3" height="200" />
    <img src="t-photos/foto_divertida.jpeg" alt="Fun team photo" height="200" />
  </p>
</details>

---

## 2. 📂 Repository Structure

Each folder has its own README describing what it contains.

<div align="center">

| 📁 Folder | 🎯 Contents | 📖 Documentation |
| :--- | :--- | :--- |
| **💾 [`src/`](src/)** | **Vehicle code**<br>• ESP32 firmware (open and obstacle challenges)<br>• Libraries and how to upload the code<br>• Link to the Android app code | [🔗 See code documentation](src/README.md) |
| **⚙️ [`models/`](models/)** | **Mechanical parts**<br>• STL files for every printed part<br>• What each part is and where it goes | [🔗 See model documentation](models/README.md) |
| **🔌 [`schemes/`](schemes/)** | **Electronics**<br>• Power and sensor schematics<br>• Current and previous versions | [🔗 See schematic documentation](schemes/README.md) |
| **🚗 [`v-photos/`](v-photos/)** | **Vehicle photos**<br>• Six views of the 2026 and 2025 vehicles | [🔗 See vehicle photos](v-photos/README.md) |
| **👥 [`t-photos/`](t-photos/)** | **Team photos**<br>• Official photo, fun photo, and team members | [🔗 See team photos](t-photos/README.md) |
| **🎥 [`video/`](video/)** | **Performance videos**<br>• Open challenge and obstacle challenge | [🔗 See videos](video/README.md) |
| **📚 [`other/`](other/)** | **Additional resources**<br>• Images of components, CAD, construction, and diagrams | [🔗 See additional resources](other/README.md) |

</div>

---

## 3. 🤖 Our Vehicle

<div align="center">
  <img src="v-photos/ADELANTE2026.jpg" alt="Prometheus 2026 vehicle" width="420">
</div>

### 3.1. 📋 General Specifications

| Subsystem | 2026 solution |
| :--- | :--- |
| **Processing** | One ESP32 for control and sensors, and an Android phone for computer vision |
| **Chassis** | Two-level monochassis designed in Onshape and printed in PETG (179 mm long, 115 mm chassis width) |
| **Drivetrain** | 220 RPM 25GA-370 motor with an interchangeable printed gear system: Configuration A (1.67:1 with 15T→25T) and Configuration B (1:1 with 20T→20T) |
| **Steering** | Ackermann steering printed in PETG-CF and driven by an MG90 servo; 40° on the inner wheel and a 176 mm turning radius |
| **Sensors** | 4 × VL53L0X ToF, BNO055 IMU, TCS34725 color sensor, and 2 × Sharp GP2Y0A21 |
| **Vision** | Our own OpenCV app that detects red and green pillars and the magenta zone |
| **Power** | 2S 2200 mAh 50C LiPo with four regulated lines (motor, control, servo, and general 5 V); the IMU and ToF sensors draw 3.3 V from the ESP32 |
| **Interface** | Start switch, NeoPixel strip, and buzzer to show status |

### 3.2. 🧩 System Architecture

```mermaid
flowchart LR
    subgraph VISION["📱 Android phone"]
        CAM["Camera"] --> APP["OpenCV app<br/>detection and CommandManager"]
    end

    subgraph SENSORES["📡 Sensors"]
        TOF["4 × VL53L0X ToF"]
        IMU["BNO055 IMU"]
        COL["TCS34725 color"]
        SHARP["2 × Sharp"]
    end

    subgraph CONTROL["🧠 ESP32"]
        FSM["State machine<br/>and PID control"]
    end

    subgraph ACTUADORES["⚙️ Actuators"]
        DRV["TB6612FNG"] --> MOT["25GA-370 motor"]
        SERVO["MG90 servo"] --> ACK["Ackermann steering"]
        UI["NeoPixel and buzzer"]
    end

    APP -- "lane commands (USB serial)" --> FSM
    TOF -- "I2C #1" --> FSM
    IMU -- "I2C #1" --> FSM
    COL -- "I2C #2" --> FSM
    SHARP -- "analog" --> FSM
    FSM -- "PWM" --> DRV
    FSM -- "PWM" --> SERVO
    FSM --> UI
```

The phone decides **what** to do about the pillars, and the ESP32 decides **how** to do it: it holds the heading and the distance to the wall, counts the lines, and drives the motor and the steering.

### 3.3. 🔄 360° Technical Inspection Gallery

These are the six required views of the vehicle. They make it possible to check symmetry, center of gravity (*CoG*) distribution, the condition of the printed chassis, and ground clearance.

| 📸 View | 🖼️ Photo | 🔍 What to check |
| :--- | :---: | :--- |
| **Front** | <img width="360" alt="Front view" src="v-photos/ADELANTE2026.jpg" /> | • Front and diagonal sensors.<br>• Steering mechanism alignment.<br>• Front height and how centered the structure is. |
| **Rear** | <img width="360" alt="Rear view" src="v-photos/DETRAS2026.jpg" /> | • Motor and rear drivetrain coupling.<br>• Stiffness of the drive axle mounts.<br>• Neatness of the power wiring. |
| **Top** | <img width="360" alt="Top view" src="v-photos/ARRIBA2026.jpeg" /> | • Side-to-side weight balance.<br>• Position of the ESP32 and the IMU.<br>• Wiring routed between levels. |
| **Bottom** | <img width="360" alt="Bottom view" src="v-photos/ABAJO2026.jpg" /> | • Smooth, flat base with no rubbing parts.<br>• Ground clearance.<br>• Screw and nut fastening. |
| **Right side** | <img width="360" alt="Right side view" src="v-photos/DERECHA2026.jpg" /> | • Access to switches and controls.<br>• Spacing between levels (drivetrain, logic, and power).<br>• Alignment of the right-side sensors and wheels. |
| **Left side** | <img width="360" alt="Left side view" src="v-photos/IZQUIERDA2026.jpg" /> | • Left-side sensors.<br>• Ventilation for the power modules. |

---

## 4. 🧠 Control, Sensing, and Interface Components

| Image | Component | Description | 🔗 Documentation |
| :----: | :--- | :--- | :--- |
| <img src="other/esp32.png" width="110" alt="ESP32"> | **ESP32 microcontroller** | The brain of the vehicle. A dual-core processor with Wi-Fi/Bluetooth and enough GPIO to read every sensor (I2C and analog) and control the actuators in real time. | • [ESP32 datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)<br>• [Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/) |
| <img src="other/sensor_infrarojo.jpg" width="110" alt="Sharp"> | **Sharp GP2Y0A21 infrared sensor** | Measures distance by infrared triangulation (10 to 80 cm). Mounted on the front diagonals to detect nearby walls and obstacles. They will be replaced by two VL53L0X sensors (🔜). | • [Sharp datasheet](https://global.sharp/products/device/lineup/data/pdf/datasheet/gp2y0a21yk_e.pdf)<br>• [Pololu guide](https://www.pololu.com/product/136) |
| <img src="other/sensor-vl53l0x-de-tof.jpg" width="110" alt="Diagonal VL53L0X"> | **🔜 2 × VL53L0X ToF sensors (diagonals)**<br><sub>Upcoming change</sub> | Will replace the two Sharp sensors on the front diagonals. They are two of the VL53L0X units removed from the front, sides, and rear: digital I2C readings instead of analog ones. See [17. Upcoming Improvements](#17--upcoming-improvements). | • [ST datasheet](https://www.st.com/resource/en/datasheet/vl53l0x.pdf)<br>• [Adafruit library](https://github.com/adafruit/Adafruit_VL53L0X) |
| <img src="other/sensor-vl53l0x-de-tof.jpg" width="110" alt="VL53L0X"> | **VL53L0X ToF sensors** | Measure distance with an infrared laser using time of flight. All four share the I2C #1 bus, and each one has its own XSHUT pin so it can be given a different address. | • [ST datasheet](https://www.st.com/resource/en/datasheet/vl53l0x.pdf)<br>• [Adafruit library](https://github.com/adafruit/Adafruit_VL53L0X) |
| <img src="https://a.pololu-files.com/picture/0J8677.600x480.jpg?2ae2631a3cf1133a512eadb1d5501cea" width="110" alt="VL53L1X"> | **🔜 3 × VL53L1X ToF sensors**<br><sub>Upcoming change</sub> | Will replace the front and side VL53L0X sensors. Range of up to 4 m (twice the VL53L0X), a 27° field of view with a programmable region of interest, and short, medium, and long distance modes. They sit on the I2C #1 bus with addresses assigned through XSHUT. See [17. Upcoming Improvements](#17--upcoming-improvements). | • [ST datasheet](https://www.st.com/resource/en/datasheet/vl53l1x.pdf)<br>• [Pololu module](https://www.pololu.com/product/3415)<br>• [Pololu library](https://github.com/pololu/vl53l1x-arduino) |
| <img src="https://a.pololu-files.com/picture/0J11640.600x480.jpg?e422665a470a49316f3488f548967503" width="110" alt="VL53L5CX"> | **🔜 VL53L5CX multizone ToF sensor**<br><sub>Upcoming change</sub> | Will replace the rear VL53L0X. It measures a 4 × 4 or 8 × 8 grid of zones with a 45° × 45° field of view and up to 4 m of range, so it provides a depth image of what is behind the vehicle. Draws about 100 mA while ranging. See [17. Upcoming Improvements](#17--upcoming-improvements). | • [ST datasheet](https://www.st.com/resource/en/datasheet/vl53l5cx.pdf)<br>• [Pololu module](https://www.pololu.com/product/3417)<br>• [SparkFun library](https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library) |
| <img src="other/MU_BNO055.jpg" width="110" alt="BNO055"> | **BNO055 IMU (9-axis)** | Combines an accelerometer, gyroscope, and magnetometer with a processor that computes absolute orientation (Euler angles or quaternions). Sits on the I2C #1 bus. | • [Bosch datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bno055-ds000.pdf)<br>• [Adafruit library](https://github.com/adafruit/Adafruit_BNO055) |
| <img src="other/sensor_color.png" width="110" alt="TCS34725"> | **TCS34725 color sensor** | RGB sensor with an infrared filter. Mounted under the vehicle to detect the track lines, alone on the I2C #2 bus so it does not load the navigation bus. | • [AMS datasheet](https://ams.com/documents/20143/36005/TCS34725_DS000390_3-00.pdf)<br>• [Adafruit library](https://github.com/adafruit/Adafruit_TCS34725) |
| <img src="other/Tira_de_LEDs_Neopixel.jpg" width="110" alt="NeoPixel"> | **NeoPixel LED strip** | Addressable RGB LEDs connected to pin D14. They show the program state, alerts, and driving mode. | • [WS2812B datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)<br>• [Adafruit library](https://github.com/adafruit/Adafruit_NeoPixel) |
| <img src="other/buzzer.jpg" width="110" alt="Buzzer"> | **Buzzer** | Piezo buzzer on pin D4. Beeps at each stage of the program, on read failures, and on alerts. | • [Arduino tone()](https://www.arduino.cc/reference/en/language/functions/advanced-io/tone/)<br>• [Piezo buzzers](https://www.sparkfun.com/products/7950) |
| <img src="other/interruptor.jpg" width="110" alt="Start switch"> | **Start switch** | Toggle switch on pin D23 using the ESP32's internal pull-up resistor. Flipping it to LOW starts the autonomous run once the vehicle is calibrated and in position. | • [ESP32 GPIO and pull-up](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html) |
| <img src="other/celular.png" width="110" alt="Phone"> | **Android phone** | Its camera, running our OpenCV app, detects the red and green pillars and the parking area, and sends commands to the ESP32 over USB. | • [App code](https://github.com/SamFlips/APP-WRO-PROMETHEUS-TEAM/tree/master) |

## 5. 🔌 Power and Actuation Components

| Image | Component | Description | 🔗 Documentation |
| :----: | :--- | :--- | :--- |
| <img src="other/bateria_lipo.png" width="110" alt="LiPo"> | **2S 50C 2200 mAh LiPo battery** | Main power source. 7.4 V nominal (8.4 V fully charged) and a 50C discharge rate to handle the motor's current peaks. | • [2S LiPo batteries](https://www.hobbywing.com/) |
| <img src="other/interruptor.jpg" width="110" alt="Main switch"> | **Main power switch** | ON/OFF rocker switch on the battery's positive lead. Cuts or enables power to the whole system. | • [KCD1 switch](https://www.sparkfun.com/products/11138) |
| <img src="other/XL6009.jpg" width="110" alt="XL6009"> | **XL6009 regulator (buck-boost)** | Set between 6 and 8 V depending on the desired speed. Keeps the motor driver supply stable even as the battery drops. | • [XL6009 datasheet](https://www.xlsemi.com/datasheet/XL6009%20datasheet.pdf) |
| <img src="other/tb6612fng.jpg" width="110" alt="TB6612FNG"> | **TB6612FNG motor driver** | Dual H-bridge. Receives direction and PWM from the ESP32 and delivers power to the motor with far less loss than the L298N we used before. | • [TB6612FNG datasheet](https://www.sparkfun.com/datasheets/Robotics/TB6612FNG.pdf) |
| <img src="other/motor.png" width="110" alt="25GA-370 motor"> | **25GA-370 DC motor (6 V to 8 V)** | 220 RPM metal gearmotor (21.3:1 gearbox) for propulsion. Also sold as JGA25-370. Powered by the XL6009 through the driver. | • [Gearmotors](https://www.pololu.com/category/60/micro-metal-gearmotors) |
| <img src="other/dsn_mini_360.jpg" width="110" alt="Mini 360"> | **3 × DSN-Mini 360 buck converters** | Switching regulators. #1 is set to 6 V and, after a 1N4007 diode, delivers about 5.2 V to the ESP32's VIN pin. #2, at 6 V, powers only the servo, and #3, at 5 V, powers the NeoPixel strip and the Sharp sensors. | • [MP2307 datasheet](https://www.monolithicpower.com/en/documentview/productdocument/index/doc_url/M/P/MP2307_r1.9.pdf) |
| <img src="other/servo_180.jpg" width="110" alt="MG90 servo"> | **MG90 180° servo** | Metal-gear servo that drives the [Ackermann steering](#94--ackermann-steering-system) via PWM (pin D13), with a travel of −50° to +48°. Powered at 6 V from Mini 360 #2. It will be replaced by a digital EMAX ES08MD (🔜). | • [MG90S datasheet](https://www.electronicoscaldas.com/datasheet/MG90S_Tower-Pro.pdf) |
| <img src="https://servodatabase.com/images/servos/emax-es08md.jpg" width="110" alt="EMAX ES08MD"> | **🔜 EMAX ES08MD digital servo**<br><sub>Upcoming change</sub> | Will replace the MG90. It is digital with metal gears: it corrects its position more often, holds its angle better, and has a smaller deadband. 2.0 kgf·cm at 6 V (1.6 at 4.8 V), 0.10 s/60° at 6 V, 4.8–6 V, 23 × 11.5 × 24 mm, and 13 g. See [17. Upcoming Improvements](#17--upcoming-improvements). | • [Spec sheet](https://shop-archive.boxtec.ch/emax-es08md-digital-metal-servo-13g-p-41148.html) |

---

## 6. ⚡ Power and Wiring Schematics

*Earlier versions of the power system are in the [2025 archive](#192--electronics-evolution).*

### 6.1. 🧾 Version 4 (current)

![Power diagram V4](schemes/Power_Supply_Diagram_FINALS.png)

```mermaid
graph TD
    %% Color definitions
    classDef bat fill:#7b1fa2,stroke:#ab47bc,stroke-width:2px,color:#fff;
    classDef sw fill:#616161,stroke:#9e9e9e,stroke-width:2px,color:#fff;
    classDef reg fill:#e65100,stroke:#ff9800,stroke-width:2px,color:#fff;
    classDef load fill:#0277bd,stroke:#29b6f6,stroke-width:2px,color:#fff;
    classDef logic fill:#00695c,stroke:#26a69a,stroke-width:2px,color:#fff;

    BAT["🔋 2S 50C 2200mAh<br>LiPo Battery<br>(7.4V, 8.4V Max)"]:::bat
    SW["🔘 Main<br>Power Switch"]:::sw

    XL["🚀 XL6009 Buck-Boost<br>Set to 6V-8V<br>(Motor)"]:::reg
    SD1["📉 DSN-Mini-360 #1<br>Set to 6V<br>(Control)"]:::reg
    DIODE["➡️ 1N4007 Diode<br>Voltage Drop"]:::sw
    SD2["📉 DSN-Mini-360 #2<br>Set to 6V<br>(Servo)"]:::reg
    SD3["📉 DSN-Mini-360 #3<br>Set to 5V<br>(General)"]:::reg

    ESP32["🧠 ESP32<br>Microcontroller<br>VIN at 5.2V"]:::load
    DRIVER["🏎️ TB6612FNG<br>Motor Driver<br>VM 6V-8V"]:::load
    MOTOR["⚙️ 25GA-370<br>DC Motor<br>6V-8V"]:::load
    SERVO["📐 MG90 180°<br>Servo Motor<br>6V"]:::load
    SENS5V["💡 NeoPixel &<br>Sharp Sensors<br>5V"]:::load
    SENS3V["📡 IMU & TOF<br>Sensors<br>3.3V Logic"]:::logic

    BAT --> SW

    SW --> XL
    SW --> SD1
    SW --> SD2
    SW --> SD3

    XL --> DRIVER
    DRIVER --> MOTOR

    SD1 --> DIODE
    DIODE --> ESP32

    SD2 --> SERVO
    
    SD3 --> SENS5V
    
    ESP32 --> SENS3V
```

### 6.2. 🔋 Power, Control, and Power Management Architecture

The **2S 2200 mAh LiPo battery** (7.4 V nominal, 8.4 V fully charged) is the only power source. It goes through a **main toggle switch**, which we chose because track vibration cannot knock it off.

From there, power splits into four independent regulated lines, plus the 3.3 V logic line that comes from the ESP32 itself. This keeps motor noise away from the control logic and prevents voltage drops (*brownouts*):

* **Drive line:** an **XL6009** regulator, set between 6 and 8 V depending on the speed we want, powers the **TB6612FNG H-bridge**, which in turn drives the **25GA-370 motor**.

* **Control line:** a **DSN-Mini 360** set to 6 V feeds a **1N4007 rectifier diode** (to prevent backfeeding when the USB cable is plugged in to upload code). After the diode's voltage drop, about 5.2 V reaches the **ESP32**'s VIN pin. At this voltage, its internal regulator has enough headroom and the board does not reset from voltage dips when the motor starts.

* **Servo line:** a second **DSN-Mini 360** set to 6 V powers only the **MG90 servo**. Isolating the servo on its own regulator is essential to keep its current peaks and electrical noise from interfering with the analog and digital readings in the rest of the system.

* **General 5 V line:** a third **DSN-Mini 360** set to 5 V supplies the peripherals that need this voltage. For now, this line powers only the **NeoPixel strip** and the two **Sharp distance sensors**.

* **3.3 V logic line:** the remaining modules and sensors (such as the IMU and the ToF sensors) are powered directly from the **ESP32**'s 3.3 V pin. This gives them a clean, stable voltage, free of the switching noise (ripple) that the Mini 360 regulators can introduce.

> [!IMPORTANT]
> **Key hardware change: L298N → TB6612FNG**
>
> Switching to the TB6612FNG solved two serious problems from the 2025 prototype. First, it eliminated the L298N's internal voltage drop, which robbed the motor of power. Second, the L298N generated interference that knocked out the microcontroller's I2C bus. To work around it, we had to use two ESP32 boards, one just for the motor. With the TB6612FNG that conflict disappeared and we went back to a single ESP32, which reduced weight, wiring, power draw, and points of failure.

> [!NOTE]
> **Design trade-off**
>
> Four independent regulators take up more space inside the printed chassis. We accepted that cost because separating power from the sensitive electronics eliminated the motor's inductive noise and gave us clean I2C readings throughout the run.

---

## 7. 📡 Sensor and Data Acquisition Schematics

*Earlier versions of the sensing system are in the [2025 archive](#192--electronics-evolution).*

### 7.1. 🧾 Version 4 (current)

![Sensor diagram V4](schemes/IO_Connection_Schematic_FINALS.png)

```mermaid
graph TD
    classDef mcu fill:#2b2b2b,stroke:#00e676,stroke-width:2px,color:#fff;
    classDef i2c1 fill:#1a237e,stroke:#448aff,stroke-width:2px,color:#fff;
    classDef i2c2 fill:#4a148c,stroke:#e040fb,stroke-width:2px,color:#fff;
    classDef adc fill:#e65100,stroke:#ff9100,stroke-width:2px,color:#fff;
    classDef gpio fill:#004d40,stroke:#00bfa5,stroke-width:2px,color:#fff;

    MCU["⚡ ESP32<br>Main Control<br>Board"]:::mcu

    IMU["🧭 BNO055<br>9-Axis IMU<br>I2C #1"]:::i2c1
    TOF1["🎯 VL53L0X<br>ToF 1<br>(Right)"]:::i2c1
    TOF2["🎯 VL53L0X<br>ToF 2<br>(Rear)"]:::i2c1
    TOF3["🎯 VL53L0X<br>ToF 3<br>(Left)"]:::i2c1
    TOF4["🎯 VL53L0X<br>ToF 4<br>(Front)"]:::i2c1

    COLOR["🎨 TCS34725<br>Color Sensor<br>I2C #2"]:::i2c2

    SHARP1["📡 GP2Y0A21<br>Sharp 1 (Right)<br>GPIO 34"]:::adc
    SHARP2["📡 GP2Y0A21<br>Sharp 2 (Left)<br>GPIO 35"]:::adc

    SERVO["⚙️ MG90<br>Servo Motor<br>Pin 13"]:::gpio
    NEO["🔵 NeoPixel<br>LED Strip<br>Pin 14"]:::gpio
    BUZZER["🔔 Piezo<br>Buzzer<br>Pin 4"]:::gpio
    SW["🔘 Program<br>Start Switch<br>Pin 23"]:::gpio
    DRIVER["🏎️ TB6612FNG<br>Motor Driver<br>25, 26, 27"]:::gpio
    XSHUT["🔀 XSHUT<br>Selectors<br>16, 17, 5, 18"]:::gpio

    MCU -- "I2C #1 (21/22)" --> IMU
    MCU -- "I2C #1 (21/22)" --> TOF1
    MCU -- "I2C #1 (21/22)" --> TOF2
    MCU -- "I2C #1 (21/22)" --> TOF3
    MCU -- "I2C #1 (21/22)" --> TOF4

    MCU -- "I2C #2 (32/33)" --> COLOR

    MCU --> XSHUT
    XSHUT -- "Pin 16" --> TOF1
    XSHUT -- "Pin 17" --> TOF2
    XSHUT -- "Pin 5" --> TOF3
    XSHUT -- "Pin 18" --> TOF4

    SHARP1 --> MCU
    SHARP2 --> MCU

    MCU --> SERVO
    MCU --> NEO
    MCU --> BUZZER
    SW --> MCU
    MCU --> DRIVER
```

**ESP32 Pin Assignment**

| Signal | Pin |
| :--- | :---: |
| I2C #1 SDA / SCL (IMU & ToFs) | 21 / 22 |
| I2C #2 SDA / SCL (Color Sensor) | 32 / 33 |
| XSHUT Right (ToF 1) | 16 |
| XSHUT Rear (ToF 2) | 17 |
| XSHUT Left (ToF 3) | 5 |
| XSHUT Front (ToF 4) | 18 |
| Motor IN1 / IN2 / PWM | 27 / 26 / 25 |
| Sharp Right / Left | 34 / 35 |
| Steering Servo | 13 |
| NeoPixel Strip | 14 |
| Program Start Switch | 23 |
| Buzzer | 4 |

### 7.2. 🔧 Sensor and Hardware Optimization

In this fourth version we reorganized the sensors to take advantage of the ESP32's two hardware I2C buses. The goal was to gain accuracy, keep any one sensor from slowing down the others, and make navigation stable inside the $3 \times 3$ meter track.

#### 7.2.1. Navigation subsystem (I2C bus #1, pins D21/D22)

* **ToF distance sensors:** they replaced the ultrasonic sensors and eliminated the short-range dead zones. To connect all four to the same bus, the ESP32 controls their **XSHUT** pins and turns them on one at a time to give each a different address (see 7.2.2). They also run at 3.3 V, so voltage dividers are no longer needed.
* **BNO055 IMU:** shares I2C bus #1. Its internal processor fuses the data and practically **eliminates the accumulated drift** we had with the previous gyroscope. At startup, the program takes the current heading as zero and calculates every turn relative to it.

#### 7.2.2. Calibration and addressing of the 4 ToF sensors (XSHUT)

The four VL53L0X sensors share I2C bus #1 (pins D21/D22), but out of the box they all answer at the same address (`0x29`). Turning them on at the same time would cause a collision. To solve this, each sensor's **XSHUT** pin goes to a different ESP32 GPIO and works as a software-controlled power switch:

```cpp
#define XSHUT_FRONTAL 18
#define XSHUT_TRASERO 17
#define XSHUT_IZQ     5
#define XSHUT_DER     16

#define ADDR_FRONTAL 0x30
#define ADDR_TRASERO 0x31
#define ADDR_IZQ     0x32
#define ADDR_DER     0x33
```

**Startup sequence (staggered power-up):**

```mermaid
flowchart TD
    A["Start:<br>all 4<br>XSHUT pins LOW<br>(off)"] --> B["Enable<br>XSHUT_FRONTAL<br>(HIGH)"]
    B --> C["Initialize<br>at factory<br>address 0x29"]
    C --> D["Reassign to<br>0x30<br>(ADDR_FRONTAL)"]
    D --> E["Enable<br>XSHUT_TRASERO<br>(HIGH)"]
    E --> F["Initialize<br>at 0x29 →<br>assign 0x31"]
    F --> G["Enable<br>XSHUT_IZQ<br>(HIGH)"]
    G --> H["Initialize<br>at 0x29 →<br>assign 0x32"]
    H --> I["Enable<br>XSHUT_DER<br>(HIGH)"]
    I --> J["Initialize<br>at 0x29 →<br>assign 0x33"]
    J --> K["All 4 sensors<br>active on the<br>same I2C bus"]
```

At each step only **one sensor** is powered at address `0x29`; the rest stay in reset (XSHUT LOW). Once a sensor has its final address, it stays on (XSHUT HIGH) and the process moves to the next one.

> [!NOTE]
> **Offset calibration:** depending on the surface, we calibrate each ToF sensor's offset individually so it reads the true distance, even though all four sensors almost always read nearly the same.

#### 7.2.3. Surface sensing subsystem (I2C bus #2, pins D32/D33)

* **TCS34725 color sensor:** detects and confirms the track lines. We put it on a **second, independent I2C bus** so its constant floor readings do not delay the ToF readings, which are what the car uses to avoid collisions. In the firmware it is read every 30 ms in its own FreeRTOS task.

#### 7.2.4. Redundant proximity and interface subsystem

* **Diagonal Sharp GP2Y0A21 sensors:** still connected to analog inputs, they provide a second proximity reading in the blind spots at the front.
* **User interface:** the NeoPixel strip and the buzzer show the robot's state (errors, successful calibration, line detected). The run starts with a **toggle switch** (pin D23); we dropped the push button because bounce and vibration triggered it by accident.

---

> [!IMPORTANT]
> **Simpler wiring and easier maintenance**
>
> Moving from HC-SR04 ultrasonic sensors to ToF sensors did more than raise the sampling rate; it also solved a physical problem. The HC-SR04 runs on 5 V while the ESP32 pins use 3.3 V, so in 2025 we had to hand-build resistor voltage dividers (we could not find commercial level shifters). They worked, but the circuit was bulky. With native 3.3 V I2C sensors that network disappeared, and the build became cleaner and easier to repair.

> [!NOTE]
> **Design trade-off: I2C bus traffic**
>
> Having several I2C sensors brought the risk of collisions and delays in the main `loop`. We solved it in two ways: in software, with sequential reads (*polling*) and addresses assigned through XSHUT; in hardware, by splitting the load across two buses (navigation and surface). To save space we did not add external *pull-up* resistors: the ones on the sensor boards were enough.

---

### 7.3. 📋 Summary of Hardware Changes

| Previous component | Current component | Rationale and main advantage |
| :--- | :--- | :--- |
| HC-SR04 ultrasonic sensor with voltage dividers | **4 × ToF sensors (I2C #1)** | No dead zones or resistor networks; everything at 3.3 V. Addresses assigned through XSHUT. |
| MPU6050 + HMC5883L magnetometer | **BNO055 IMU (I2C #1)** | Drift practically eliminated on the $3 \times 3$ m track. |
| Color sensor on the shared bus | **TCS34725 (isolated I2C #2)** | Its own bus, with no bottlenecks against the distance sensors. |
| Start push button | **Toggle switch (D23)** | Track vibration cannot trigger it. |
| L298N + second microcontroller | **TB6612FNG + a single ESP32** | No voltage drop and no I2C interference. |

---

## 8. 🔋 Power Management and Battery Life

In this section we calculate the vehicle's power consumption and justify the battery we chose.

### 8.1. 📊 Total Power Consumption

We estimated each component's current draw in three scenarios, using typical datasheet values: **idle** (powered but not working), **nominal** (normal operation during a lap), and **peak** (maximum instantaneous draw, such as motor startup or the LEDs at full brightness).

| Component | Qty | Idle (mA) | Nominal (mA) | Peak (mA) |
| :--- | :---: | :---: | :---: | :---: |
| **ESP32** (dual core, Wi-Fi/BT off) | 1 | ~20 | ~80 | ~80 (heavy processing) |
| **Sharp GP2Y0A21 infrared sensor** | 2 | 10 each | 30 each | 33 each |
| **VL53L0X ToF sensor** | 4 | 5 each | 15 each | 20 each (ranging) |
| **BNO055 IMU** | 1 | ~1 | 12.3 | ~16 |
| **TCS34725 color sensor** | 1 | ~0 (sleep, µA) | 3 | 3.6 |
| **NeoPixel strip** (10 LEDs) | 10 | 0 (off) | 300 (mixed colors) | 600 (100 % white) |
| **Buzzer** | 1 | 0 | 30 | 30 |
| **XL6009 regulator** (own consumption) | 1 | 15 | 15 | 15 |
| **TB6612FNG driver** (logic) | 1 | 1 | 2 | 6 |
| **25GA-370 DC motor** | 1 | 0 (stopped) | 500 | ~1800 (startup / stall) |
| **DSN-Mini 360 buck converter** (own consumption) | 3 | 5 each | 5 each | 5 each |
| **MG90 servo** | 1 | 10 (holding position) | 200 (moving) | ~700 (under load) |
| **ESTIMATED TOTAL** | | **~102 mA** | **~1277.3 mA** | **~3411.6 mA** |

🔋 **Nominal current:** ~1.28 A · **Theoretical simultaneous peak:** ~3.41 A

> [!NOTE]
> **About the peak scenario**
>
> The ~3.41 A figure is the extreme case in which *every* component hits its maximum at the same time (motor startup, servo under load, and LEDs at full white). That is very unlikely during a real lap, but it is the value we use to check the battery's minimum discharge rate (C) (see [8.4](#84--power-source-justification)).

---

### 8.2. ⏳ Battery Life Estimate

To estimate continuous run time, we use the ratio between battery capacity and calculated consumption.

#### 8.2.1. General formula

$$\text{Battery life (hours)} = \frac{\text{Battery capacity (mAh)}}{\text{Total system current (mA)}}$$

#### 8.2.2. Applied to our vehicle

$$\text{Battery life} \approx \frac{2200 \text{ mAh}}{1277.3 \text{ mA}} \approx 1.72 \text{ hours}$$

* **Estimated time:** about **1 hour and 43 minutes** of continuous operation under normal conditions.
* **Practical note:** this is a theoretical model. On the track, the motor's draw changes constantly (it rises close to stall) and the sensors work in bursts, so actual battery life depends on what the firmware is doing.

---

### 8.3. 🔁 DC-DC Converter Efficiency and Actual Battery Life

The previous calculation assumes that each component's current is the same as the current leaving the battery. That is not the case: the XL6009 and DSN-Mini 360 regulators output voltages different from the battery's (7 V for the motor, 6 V for control and the servo, and 5 V for the general line, versus 7.4 V nominal), and no conversion is 100 % efficient.

**Reference efficiencies (manufacturer datasheets):**

| Converter | Role in the system | Efficiency used | Manufacturer range |
| :--- | :--- | :---: | :---: |
| **XL6009** (buck-boost) | Drive line (motor + TB6612FNG) | ~88 % | Up to 94 % under ideal conditions; with a ratio close to 1:1 and ~500 mA load, the real value is lower |
| **DSN-Mini 360** ×3 (buck, MP2307) | Control line (ESP32), servo line, and general 5 V line | ~90 % | 90–96 % typical at moderate load |

**Actual current drawn from the battery (power balance per line):**

$$P_{\text{in}} = \frac{P_{\text{out}}}{\eta} \qquad I_{\text{battery}} = \frac{P_{\text{in}}}{V_{\text{battery}}}$$

| Power line | V out | I out (nominal) | P out | η | P in | I from battery (7.4 V) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| Drive (XL6009 → motor + TB6612FNG) | 7 V | 502 mA | 3.51 W | 88 % | 3.99 W | **≈ 540 mA** |
| Control (Mini 360 #1 → 1N4007 diode → ESP32, which supplies 3.3 V to the IMU, the 4 ToF sensors, the color sensor, and the buzzer) | 6 V | 185.3 mA | 1.11 W | 90 % | 1.24 W | **≈ 167 mA** |
| Servo (Mini 360 #2 → MG90 servo) | 6 V | 200 mA | 1.20 W | 90 % | 1.33 W | **≈ 180 mA** |
| General 5 V (Mini 360 #3 → NeoPixel strip and 2 Sharp sensors) | 5 V | 360 mA | 1.80 W | 90 % | 2.00 W | **≈ 270 mA** |
| **TOTAL ACTUAL BATTERY DRAW** | | | | | | **≈ 1157 mA** |

$$\text{Actual battery life} \approx \frac{2200 \text{ mAh}}{1157 \text{ mA}} \approx 1.90 \text{ hours} \; (\approx 1\text{ h }54\text{ min})$$

> [!NOTE]
> **The control line diode.** The control line current is calculated at the output of Mini 360 #1 (6 V). That way it already includes the 1N4007 diode drop (about 0.8 V × 185 mA ≈ 0.15 W), which is the price of protecting the ESP32 when the USB cable is plugged in. The 3.3 V loads go through the ESP32's internal regulator, which is linear: it draws the same current from its input that it delivers.

> [!IMPORTANT]
> **Why actual battery life comes out higher than the first estimate**
>
> You would expect regulator losses to *reduce* battery life, and compared with an ideal case (100 % efficiency) they do. But the first model (~1.72 h) had an error in the opposite direction: it added up each component's mA as if everything ran at battery voltage (7.4 V), when most of it runs at 5 V or 6 V. Stepping the voltage down lowers the equivalent current on the battery side, and that effect outweighs the regulator losses. The result (~1 h 54 min) matches what we observed: during a full afternoon and evening of testing, the battery never reached its minimum charge level.

---

### 8.4. ✅ Power Source Justification

#### 8.4.1. 🔋 Why LiPo?

We chose a **lithium polymer (LiPo)** battery for its **power-to-weight ratio** and its **ability to deliver a lot of current instantly** (discharge rate). Both matter for the robot's dynamic performance.

#### 8.4.2. 📊 Technology comparison

| Technology | Energy density | Discharge rate (C) | Weight / volume | With motors and servos |
| :--- | :---: | :---: | :---: | :--- |
| **LiPo (2S 50C)** | **High** | **Very high (~50C)** | **Very light** | **Excellent.** Delivers current instantly without voltage drops. |
| **Li-ion (18650)** | Very high | Low (1C to 2C) | Light | **Poor.** Current peaks pull the voltage down (*voltage sag*) and reset the ESP32. |
| **NiMH (6 cells)** | Medium | Medium | Very heavy | **Acceptable but impractical.** The extra weight costs the robot agility. |
| **Power bank (USB)** | High | Limited (2 A to 3 A) | Medium | **Incompatible.** Its overcurrent protection shuts it off. |

#### 8.4.3. Justification against the alternatives

**a) Versus Li-ion (18650).** 18650 cells have very good energy density, but their typical discharge rate is low. The motor and servo create current peaks when they start or are under load, and a standard Li-ion cell would suffer a sharp voltage drop (*voltage sag*). That would cause **ESP32 resets** or trip the battery protection (BMS). The LiPo delivers that current instantly without the voltage dropping.

**b) Versus NiMH.** NiMH batteries are safe and durable, but very heavy and bulky. Matching the **7.4 V** of a 2S LiPo would take **6 cells in series** (1.2 V each). That weight would double or triple the robot's mass, increase the motor's power draw, and reduce acceleration on the track.

**c) Versus commercial power banks.** A power bank boosts its cells to 5 V through a regulator with a strict current limit (usually 2 A or 3 A). Our simultaneous draw easily exceeds that limit:

* **10 NeoPixel LEDs** at full white: `~600 mA`
* **MG90 servo** under load: `~700 mA`
* **25GA-370 motor** at startup or under load: `> 1 A`

Once that limit is exceeded, the power bank shuts off. In addition, the **25GA-370** motor performs best between **6 V and 8 V**; at 5 V it would lose torque and speed.

#### 8.4.4. Why 2200 mAh and not more capacity?

We chose 2200 mAh based on three criteria: weight, discharge rate, and a battery life we have already confirmed in practice.

- **Weight:** the 2S 2200 mAh 50C LiPo we use weighs **115 g**. A 2600 or 3000 mAh pack of the same technology is proportionally heavier, and that weight affects the torque-to-weight ratio (see [section 11](#11--speed-torque-and-traction)) without giving us any benefit, because the current battery life already covers a competition round with plenty to spare.
- **Discharge rate (50C):** with 2200 mAh and 50C, the battery can deliver up to 110 A instantly, far above the system's real peak (~3.4 A, see [8.1](#81--total-power-consumption)). The motor starts without the voltage dropping and without affecting the control logic.
- **Proven battery life:** according to the recalculation with losses ([8.3](#83--dc-dc-converter-efficiency-and-actual-battery-life)), actual battery life is ~1 h 54 min. During a full afternoon and evening of testing we never reached the charge limit, so 2200 mAh leaves plenty of margin to practice and compete without carrying dead weight.

> [!NOTE]
> **Design trade-off**
>
> Going up to 2600–3000 mAh would have added about 20–35 g (estimated proportionally within the same product line) in exchange for battery life that is not what limits us. The real bottleneck is the time the avoidance algorithm takes (see the lap times in [11.4](#114--theoretical-vs-actual-lap-time)), not the available energy. We prefer to keep the chassis as light as possible.

---

## 9. 🧱 Structural Design, Drivetrain, and Additive Manufacturing

*The design and construction of the 2025 chassis are in the [2025 archive](#193--chassis-construction).*

### 9.1. 📝 Redesign Approach and Concept Prototyping

We started by analyzing what failed in last year's rounds. The main conclusion was that the chassis we carried over was too long: it hurt cornering, increased the turning radius, and made the car less nimble when avoiding obstacles.

To fix this without taking space away from the electronics, we set a geometric target: a **1:1 aspect ratio**, meaning the distance between the front and rear axles (*wheelbase*) should equal the vehicle's width (track). That makes the steering more responsive.

> [!IMPORTANT]
> **Drivetrain: from belts to gears**
>
> Before the final model, we tested the idea with quick prototypes. We first considered a timing belt drive, but we could not get belts in the right size and there was a risk of *slipping*, so we dropped it. We settled on parallel spur gears, which let us rotate the motor mount, make the drivetrain more compact, and confirm that the chassis could be shortened.

#### 9.1.1. Early concept prototypes

<p align="center">
  <img src="other/PrototipoTinkercad2026.jpeg" alt="Concept prototype 1" height="210">
  <img src="other/PrototipoTinkercad2_2026.jpeg" alt="Concept prototype 2" height="210">
  <img src="other/prototipoComparado2026.jpeg" alt="Proportion comparison" height="210">
</p>

---

### 9.2. 🚙 Moving to Professional CAD and the Monochassis Design

With the geometry validated, we moved to **Onshape**, a parametric CAD environment. The main innovation is the lower-level **Monochassis**: a single part that integrates the mounts, wiring channels, and component housings. This eliminated multi-part assemblies and improvised fasteners that loosened with vibration.

The platform measures **179 mm long by 115 mm wide**, with a wheelbase of **~112 mm**, which meets the 1:1 ratio we were aiming for.

<table>
<tr>
<td align="center" width="50%"><img src="other/chasis_lego_supe_cad.png" alt="Previous Monochassis, top view" style="width: 100%; max-width: 340px; aspect-ratio: 1/1; object-fit: contain;"><br><sub><b>Previous version (top view):</b> front cutout for the LEGO steering and no stops for the cable tubes.</sub></td>
<td align="center" width="50%"><img src="other/chasis_ackerman_supe_cad.png" alt="New Monochassis, top view" style="width: 100%; max-width: 340px; aspect-ratio: 1/1; object-fit: contain;"><br><sub><b>Current version (top view):</b> solid base for the Ackermann steering and central holes with a lip for the cable tubes.</sub></td>
</tr>
<tr>
<td align="center" width="50%"><img src="other/chasis_lego_inf_cad.png" alt="Previous Monochassis, bottom view" style="width: 100%; max-width: 340px; aspect-ratio: 1/1; object-fit: contain;"><br><sub><b>Previous version (bottom view):</b> flat underside with no wiring guides or LED channels.</sub></td>
<td align="center" width="50%"><img src="other/chasis_ackerman_inf_cad.png" alt="New Monochassis, bottom view" style="width: 100%; max-width: 340px; aspect-ratio: 1/1; object-fit: contain;"><br><sub><b>Current version (bottom view):</b> recessed channels for the NeoPixel strip, a route for the color sensor wiring, and exits to the cable tubes.</sub></td>
</tr>
</table>

> [!IMPORTANT]
> **Monochassis evolution: Ackermann, integrated wiring, and ToF pedestals**
>
> When we moved from the LEGO-steering prototype to the final design, we took the opportunity to make key improvements on both the top and bottom of the first level:
> * **Front base for Ackermann steering:** we replaced the original LEGO-block-shaped cutout with a solid structural base that holds the servo mount, integrates the angled lugs for the knuckle pivots (M3 *kingpins*), and includes curved relief cuts so the screws do not rub when turning.
> * **Raised mounts for the ToF sensors:** in early testing, the lower distance sensors sat flush with the track, which caused reading errors from reflected light. To fix this, we built hollow U-shaped pedestals directly into the chassis that raise the ToF sensors to their optimal height. This mechanical improvement worked so well that we standardized it as a universal modular system for the rest of the robot (using two M3 through-bolts).
> * **Recessed underside channels:** on the underside, we modeled a perimeter U-channel to hold and protect the **NeoPixel strip**, plus a dedicated curved channel that guides the wires from the front housing of the **TCS34725 color sensor** so they are not exposed to rubbing on the track.
> * **Lipped holes for cable tubes:** both underside channels lead to two central holes that go through the chassis. On the top side, these holes have a small perimeter lip designed to hold a **cable tube** in each one, so all the wires run vertically, straight, and neatly up to the second level.

<p align="center">
  <img src="other/Isometrica_Monochasis2026.png" alt="Isometric view of the 2026 Monochassis" width="700"><br>
  <sub><b>Isometric view of the final Monochassis:</b> it shows the platform's three-dimensional layout, the support towers, the steering base, and the drivetrain compartment, ready for component installation.</sub>
</p>


#### 9.2.1. Modular drivetrain (torque vs. speed)

* **Drive motor:** 220 RPM 25GA-370 (JGA25) with a metal gearbox. The Monochassis has a recessed cradle that keeps its shaft perfectly aligned with the drivetrain.
* **Interchangeable gears (pinion and spur gear):** we redesigned the rear of the chassis and enlarged the axle cutout. This lets us fit two configurations without modifying the main structure, depending on the track:
  * **Configuration A (15T → 25T):** 1.67:1 reduction. Gives up top speed to maximize torque.
  * **Configuration B (20T → 20T):** direct 1:1 ratio. Uses the motor's spare torque to reach a higher top speed.
* **Rear axle and adapter sleeves:** since we could not find 4 mm precision rod, we used the smooth core of welding electrodes (~2.4 mm). To make up the diameter difference with the bearings, we designed and printed **3D adapter sleeves (`Bearing_Shaft_Adapter_Sleeve.stl`)**. These parts wrap the rod and *press-fit* into the bearing, giving a clean, rigid centering.
* **Bearings:** the axle turns on 604ZZ bearings (4 × 12 × 4 mm) seated in housings built into the chassis.

---

### 9.3. 📏 Tolerance Validation and Virtual Assembly

Before putting everything together, we modeled and reviewed each mechanical part separately in Onshape to make sure the tolerances were correct.

#### 9.3.1. Key components and fastening systems

We designed our own gears, each with a fastening method suited to its shaft:

* **Drive pinion ("D" shaft):** to couple the 15-tooth gear to the motor, we used the flat on its shaft. The bore has a "D" profile with tolerances designed for 3D printing, so it *press-fits* onto the shaft. It transmits full torque without wearing and does not work loose from vibration.
* **Spur gear (smooth rod and flat spot):** in the original configuration (25T), we used a collar with a "C" clamp, but during testing we noticed the gear slipped on the smooth rod when the motor applied sudden torque. To solve this for good, we made two changes together:
  1. We switched to a design with a **set screw** in the collar.
  2. We machined the rear rod, grinding it down slightly (*flat spot*) right where the spur gear's set screw hole sits, to create a **flat face**.

  Tightening the set screw against that flat face instead of the smooth curve locks the spur gear in place completely.

<table>
<tr>
<td align="center" width="50%"><img src="other/Engranaje_15D.png" alt="15-tooth pinion" width="300"><br><sub><b>Config A:</b> drive pinion (15T) with a "D" coupling.</sub></td>
<td align="center" width="50%"><img src="other/Engranaje_25D.png" alt="25-tooth spur gear" width="300"><br><sub><b>Config A:</b> old spur gear (25T) with a "C" clamp.</sub></td>
</tr>
<tr>
<td align="center" width="50%"><img src="other/Engranaje_20D_motor.png" alt="20-tooth pinion" width="300"><br><sub><b>Config B:</b> drive pinion (20T) with a "D" coupling.</sub></td>
<td align="center" width="50%"><img src="other/Engranaje_20D_prisionero_varilla.png" alt="20-tooth spur gear with set screw" width="300"><br><sub><b>Config B:</b> redesigned spur gear (20T) with a set screw.</sub></td>
</tr>
</table>

#### 9.3.2. Drive coupling: LEGO wheel adapter

Last year's **LEGO wheels** have very good grip on the track, so we kept them. To attach them to the new metal drivetrain, we completely redesigned the 3D-printed adapters, solving two critical problems: slipping and the structural weakness of the print layers.

1. **Rod coupling (set screw and flat spot):**
We moved away from the old "C" clamp. The adapters now attach to the rod with a **set screw**. To guarantee full traction without slipping, we **ground flat spots on both ends of the metal rod**. The set screw presses directly on that flat face, locking the part 100 %.

2. **Wheel coupling (original LEGO insert):**
In the previous adapter, the LEGO cross profile was printed directly in the plastic. Because of the direction of the print layers (anisotropy), this part was brittle and tended to snap from side impacts or drops. In the new design, we removed the printed cross and left a hollow bore instead. We *press-fit* an **original LEGO cross axle** into it. It fits exactly, does not come loose, and gives us the much higher strength of LEGO's injection-molded plastic.

<table>
<tr>
<td align="center" width="50%"><img src="other/adaptador_lego.png" alt="LEGO axle adapter (old)" width="300"><br><sub><b>Previous version:</b> friction fit (a "C" clamp) and a 3D-printed LEGO cross (brittle).</sub></td>
<td align="center" width="50%"><img src="other/adaptador_prisionero.png" alt="Adapter with set screw (new)" width="300"><br><sub><b>Current version:</b> set screw against the flat spot, with an original LEGO axle press-fit insert.</sub></td>
</tr>
</table>



#### 9.3.3. Virtual assembly and failure prevention

With the parts validated, the full virtual assembly helped us find interferences and correct the design before printing.

> [!TIP]
> **Motor mount redesign**
>
> In the simulation, we saw that the screws holding the motor were blocked by the spur gear on the rear axle. Removing the motor would have meant taking off the rod and almost the entire drivetrain, which takes a long time in the pits. We moved the holes to the free corners of the mount, and now the motor comes out by removing only the pinion.

<table>
<tr>
<td align="center" width="50%"><img src="other/subensamble_engranajes.png" alt="Motor subassembly with gears and rod" width="320"><br><sub>Interference check of the gear train.</sub></td>
<td align="center" width="50%"><img src="other/motorConEngranajesOneShape2026.jpeg" alt="Rear of the chassis with the drivetrain installed" width="320"><br><sub>Drive block ready to integrate.</sub></td>
</tr>
</table>

---

### 9.4. 🛞 Ackermann Steering System

The LEGO steering we carried over from 2025 was **parallel**: both wheels turned the same angle, it had little maximum steering angle, and it was press-fit into a cutout in the chassis, with play. We replaced it with **Ackermann steering designed in Onshape and printed in PETG-CF**: a servo moves a flat link, the link moves a tie rod, and the tie rod turns both steering knuckles, each on a metal M3 pivot (*kingpin*).

<div align="center">

| 🛞 Inner wheel angle | 🔄 Minimum turning radius | 🎯 Max. error vs. ideal | 📏 Min. chassis clearance | ⚙️ Servo travel |
| :---: | :---: | :---: | :---: | :---: |
| **40°** | **176 mm** | **1.4°** | **1.00 mm** | **98°** |

</div>

<table>
<tr>
<td align="center" width="50%"><img src="other/direccion_ackermann_superior.jpg" alt="Ackermann steering, top view" width="380"><br><sub><b>Top view.</b> Both steering knuckles with their pivots and the tie rod. The servo sits in its mount (in purple), in front of the tie rod.</sub></td>
<td align="center" width="50%"><img src="other/direccion_ackermann_perspectiva.jpg" alt="Ackermann steering in perspective" width="380"><br><sub><b>Perspective from above and in front.</b> The link connects the servo horn to the tie rod. It looks asymmetric because the tie rod has no center hole for the link: the joint sits 14 mm from the center.</sub></td>
</tr>
</table>

<p align="center">
  <img src="other/direccion_ackermann_animacion.gif" alt="Animation of the Ackermann steering travel in Onshape" width="760"><br>
  <sub><b>Onshape animation.</b> The servo sweeps its CAD range (from −46.8° to +34.3°), showing how the link moves the tie rod and how the inner wheel turns more than the outer wheel in each direction.</sub>
</p>

#### 9.4.1. Starting data

| Parameter | Value |
| :--- | :--- |
| Wheelbase (L) | 112 mm |
| Chassis width | 115 mm |
| LEGO tire / rim | #44309 (43.2 × 22 mm) / #56145 (30.4 × 20 mm) |
| Rear track | 140 mm outside to outside (118 mm between tire centers) |
| Front width (as built) | ≈ 143 mm from wheel edge to wheel edge (≈ 121 mm between centers), the same as the rear axle |
| Wheel adapter | 25.7 mm long (13.7 mm in the previous version) |
| Plate thickness / spacing between levels | 4 mm / 60 mm |
| Wheel axle height | 10.75 mm above the plate (the same as the rear axle) |
| Servo | **Upcoming:** EMAX ES08MD (digital, 23 × 11.5 × 24 mm, 13 g, 4.8–6 V, 2.0 kgf·cm at 6 V; see [17. Upcoming Improvements](#17--upcoming-improvements)).<br>**In testing:** MG90 (23 × 12.2 mm) with its stock horn, using the hole 14 mm from the shaft. |
| Material | PETG-CF (knuckles, tie rod, link, and mount) |

#### 9.4.2. The 1:1 ratio: the 2026 chassis vs. the 2025 chassis

Steering geometry depends directly on the wheelbase (L): at the same wheel angle, the turning radius grows in proportion to L. That is why the first step of the redesign was to shorten the chassis until the wheelbase was nearly equal to the width (a 1:1 ratio). Measurements taken from the 3D models of both chassis:

| Parameter | 2025 chassis | 2026 Monochassis | Change |
| :--- | :---: | :---: | :---: |
| **Chassis length** | 242 mm | 179 mm | 🔻 −63 mm (−26 %) |
| **Chassis width** | 116 mm | 115 mm | ≈ the same |
| **Length / width ratio** | ≈ 2.1 : 1 | ≈ 1.56 : 1 | More compact |
| **Wheelbase / width** | Chassis lengthened for the tripod | 112 / 115 ≈ **0.97 : 1** | ≈ 1:1 |

$$R = \frac{L}{\tan\delta_i} + \frac{K}{2} \quad\Rightarrow\quad \text{at the same steering angle, a shorter } L \text{ means a smaller radius}$$

- **Tighter turns:** with L = 112 mm, the steering reaches a 176 mm radius. On the long 2025 chassis, the same angle would have produced a larger radius, because the first term of the formula grows with L.
- **Same space for the electronics:** the width did not change; what we cut was the extra length that only existed to balance the tripod.
- **No flexing:** shortening the chassis eliminated the lever effect that bent it in the middle, and the lead counterweights are no longer needed (see [12. Engineering Log](#12--engineering-log-chassis-evolution-v0--v2)).
- **Higher K/L ratio:** with K = 84 mm and L = 112 mm, K/L = 0.75. This is the figure that sets how much more the inner wheel must turn than the outer wheel (formula 1).

#### 9.4.3. Operating principle and formulas

In Ackermann steering, the extended axes of the two front wheels meet at a point on the line of the rear axle. That way, each wheel follows its own circle without being dragged sideways. To achieve this, the inner wheel has to turn more than the outer wheel.

```mermaid
flowchart LR
    S["🔧 MG90 servo<br/>(14 mm horn)"] --> B["Flat link<br/>14.5 mm"]
    B --> T["Tie rod<br/>68.6 mm"]
    T --> MI["Left knuckle<br/>22 mm arm at 20.56°"]
    T --> MD["Right knuckle<br/>22 mm arm at 20.56°"]
    MI --> RI["🛞 Left wheel<br/>(M3 kingpin)"]
    MD --> RD["🛞 Right wheel<br/>(M3 kingpin)"]
```

| # | Formula | Use |
| :---: | :--- | :--- |
| (1) | $\cot\delta_e - \cot\delta_i = K / L$ | Ackermann condition |
| (2) | $\theta = \arctan\left(\frac{K/2}{L}\right)$ | Steering arm angle |
| (3) | $s = K - 2a\sin\theta$ | Tie rod hole spacing |
| (4) | $R = \frac{L}{\tan\delta_i} + \frac{K}{2}$ | Turning radius (to the center of the rear axle) |
| (5) | $\frac{\delta_i - \delta_{e,\text{real}}}{\delta_i - \delta_{e,\text{ideal}}} \times 100$ | Ackermann percentage |
| (6) | $F = \tau / r_{\text{horn}}$ | Force at the servo horn |

Where $\delta_i$ and $\delta_e$ are the inner and outer wheel angles, $K$ is the distance between the *kingpin* axes, $L$ is the wheelbase, and $a$ is the steering arm length.

#### 9.4.4. Choosing the pivot spacing (K)

The first calculation used the chassis width (115 mm) as $K$, and that was a mistake: $K$ is the distance between the pivots, not between the wheels. We compared four options with L = 112 mm and a 22 mm arm:

| K (mm) | Criterion | θ | Hole spacing | Decision |
| :---: | :--- | :---: | :---: | :--- |
| 115 | Chassis width (initial assumption) | 27.2° | — | ❌ Not the distance between pivots |
| 96 | Kingpin at the inner face of the tire | 23.2° | 78.7 mm | ❌ The Ø10 lugs intruded about 5 mm into the rim |
| 108 | Kingpin 5 mm from the wheel's center plane | 25.7° | 88.9 mm | ❌ The knuckle would have to fit inside the rim cavity |
| **84** | **Kingpin 6 mm inside the inner face (x = ±42 mm)** | **20.56°** | **68.55 mm** | ✅ **Adopted** |

#### 9.4.5. Steering arm, tie rod, and ideal angles

With K = 84 mm:

$$\theta = \arctan\left(\frac{42}{112}\right) = 20.56^\circ \qquad s = 84 - 2 \cdot 22 \cdot \sin(20.56^\circ) = 68.55 \text{ mm}$$

With a 22 mm arm, the tip sits 7.72 mm toward the center and 20.60 mm behind the kingpin. The tie rod was built with 68.6 mm between holes and a third hole 14 mm from the center for the link.

Applying formula (1) with K/L = 0.75:

| Inner δ | Ideal outer δ | Turning radius | Error of a parallel steering |
| :---: | :---: | :---: | :---: |
| 10° | 8.85° | 677 mm | 1.15° |
| 20° | 15.96° | 350 mm | 4.04° |
| 30° | 21.95° | 236 mm | 8.05° |
| 40° | 27.24° | 176 mm | 12.76° |

The last column is the difference between the ideal and an outer wheel that turns as much as the inner one (parallel steering). It grows with the angle, which is why parallel steering is only acceptable for small turns.

#### 9.4.6. Servo and link: what the CAD predicted and what happened in the build

The servo is mounted **upside down** (shaft pointing down), in front of the tie rod and resting on the plate, with a 14 mm horn and a 14.5 mm flat link. This keeps the servo horn, the link, and the tie rod working in almost adjacent planes. An earlier design with a Z-shaped link and a 25 mm height offset was dropped because it levered against the joints and created play. We also required the angle between the horn and the link to never drop below 40°, to avoid dead center; the first servo range (−19.6° to +24.8°) did not meet that.

| Item | CAD | Actual build |
| :--- | :---: | :---: |
| Servo range | −46.8° to +34.3° | **−50° to +48°** |
| Total travel | 81.1° | **98°** |
| Center of the window | −6.2° | −1° |
| Servo horn / link | 14 mm / 14.5 mm | 14 mm (outermost hole of the MG90 horn) / 14.5 mm |

The CAD was optimistic: the tie rod needs 22 mm of travel at the link joint, and with a 14 mm horn that takes 100° to 104° of servo rotation. The asymmetry in the CAD came from the off-center joint between the link and the tie rod (14 mm from the center) and from the lateral offset of the servo shaft. On top of that, the servo had been mounted backward; once we flipped it and widened the range, the steering reached the expected angles and the travel became almost symmetric.

> [!NOTE]
> The degrees in the code depend on the µs-per-degree calibration (we started from 10 µs/°). It is worth checking with a protractor that the horn turns as much as it is told to.

**Servo torque:** with 2.0 kgf·cm (0.196 N·m) and a 14 mm horn, the servo pushes with about **14 N**. The estimated load to steer with the car standing still, with about 0.66 kg on the front axle, is less than 0.3 kgf·cm: a margin of **more than 6×**. The weak point of the system is mechanical play, not force.

#### 9.4.7. Vertical layout

Heights are measured from the underside of the plate. The wheel extends 6.85 mm below the plate, so no part can sit closer than about 2 mm to the ground.

| Part | Height z (mm) |
| :--- | :---: |
| Bottom of the tire (ground) | −6.85 |
| Knuckle foot | −4.5 to −0.5 (2.35 mm above the ground) |
| Plate and kingpin bridges | 0 to 4 |
| Knuckle lug and steering arm | 4.5 to 7.5 |
| Tie rod | 8 to 11 |
| Link | 11.5 to 14.5 |
| Servo horn | 15 to 17 |
| Wheel axle | 14.75 |

#### 9.4.8. Interference check in CAD

We modeled a "ghost wheel" (a Ø43.2 × 14 mm cylinder with its axle at z = 14.75 mm) and rotated it around the kingpin in 5° steps to look for collisions with the chassis, the knuckles, the tie rod, and the link.

| Problem found | Fix | Result |
| :--- | :--- | :--- |
| At 40°, the inner rear corner of the tire rubbed against the bridge and the shoulder wall | Angled bridge and cutouts in the shoulder between x = 33.3 and 37.5 mm | Minimum clearance of 1.00 mm across the full range |
| A new cutout damaged the steering arm | The cutouts were limited to the chassis | Part restored |
| The link bushings did not clamp the stack of parts | 4 mm bushings (z 11–15) | Firm joints and a free link (0.5 mm) |
| The tie rod touched the servo's M3 screw head, and the tie rod and link touched the pillars at −40° | Relief in the tie rod and a recess in the pillars | All clearances ≥ 1 mm |

**Criterion:** ≥ 1 mm against all other parts, and 0.5 mm only within a single joint. The 0.5 mm clearances between the wheel and the knuckle are static, because both turn together.

#### 9.4.9. Result: actual vs. ideal steering

<table>
<tr>
<td align="center" width="50%"><img src="other/direccion_ackermann_giro_izquierda.jpg" alt="Steering turned fully to the left" width="380"><br><sub><b>Full left turn.</b> The left (inner) wheel turns more than the right one.</sub></td>
<td align="center" width="50%"><img src="other/direccion_ackermann_giro_derecha.jpg" alt="Steering turned fully to the right" width="380"><br><sub><b>Full right turn.</b> Now the right wheel is the inner one, and it turns the most.</sub></td>
</tr>
</table>

Angle pairs from the mechanism model (rigid 68.6 mm tie rod):

| Inner δ | Ideal outer δ | Outer δ of our mechanism | Error | % Ackermann |
| :---: | :---: | :---: | :---: | :---: |
| 10° | 8.85° | 9.25° | +0.40° | 65 % |
| 20° | 15.96° | 17.06° | +1.10° | 73 % |
| 30° | 21.95° | 23.37° | +1.42° | 82 % |
| 40° | 27.24° | 27.96° | +0.72° | 94 % |

```mermaid
xychart-beta
    title "Error vs. ideal Ackermann (degrees)"
    x-axis ["10°", "20°", "30°", "40°"]
    y-axis "Error (°)" 0 --> 14
    bar [1.15, 4.04, 8.05, 12.76]
    line [0.40, 1.10, 1.42, 0.72]
```

<sub>Bars: parallel steering (like the LEGO one). Line: our Ackermann steering.</sub>

The error never exceeds 1.4°, and the mechanism gets closest to the ideal exactly where it matters most: at full lock (94 %). At small angles, the absolute error is under 0.5°.

**Measurement on the actual build** (protractor on paper, servo at its −50° and +48° limits):

| Turn | Inner wheel | Outer wheel | Ideal outer | Error | % Ackermann |
| :--- | :---: | :---: | :---: | :---: | :---: |
| Right | 40° | 30° | 27.24° | +2.8° | 78 % |
| Left | 40° | 25° | 27.24° | −2.2° | 118 % |
| **Average** | **40°** | **27.5°** | **27.24°** | **+0.3°** | **98 %** |

The average outer wheel angle (27.5°) matches the ideal (27.2°) and the model (28.0°). The 5° difference between sides is not explained by the mechanism, which is symmetric: the most likely cause is a 2° to 3° zero offset, which is the uncertainty of a protractor on paper. The total travel of each wheel (65° and 70°, 67.5° on average) matches the model's ≈ 68°.

#### 9.4.10. Adjustments after physical testing

- **Servo flipped:** it had been mounted backward. After correcting it and widening the range to −50°/+48°, the steering reaches the expected angles.
- **Vertical clearances** of the knuckle foot and lug, and of the bushings: from 0.5 mm to 0.1 mm per side. This eliminated the vertical play; the dead spots that appeared were solved by the corrected servo and the wider range.
- **A channel of about 2 mm** on the top face of the chassis for the screw heads at the tips of the steering arms, which were rubbing.
- **M3 pivot with a countersunk head.** We could not get shoulder bolts, so the nuts on the standard screws are secured with lock nuts or thread locker.
- **Wheel adapter lengthened** from 13.7 to 25.7 mm: the width from wheel edge to wheel edge is now ≈ 143 mm, the same as the rear axle.
- **Wheel axle:** Ø2.6 mm welding rod with a printed bearing surface instead of 604ZZ bearings, and a LEGO cross joint seated in a 6 mm recess in the adapter.

**Effect of the 143 mm width:** the Ackermann geometry does not change, because it depends on K = 84 mm and L = 112 mm, not on the track. The wheel center sits about 1.5 mm farther out on each side, which increases the steering effort by 9 % (negligible against the margin of more than 6×) and moves the tire 1.5 mm away from the knuckle and the chassis. With the front track equal to the rear track, the rear wheels follow the front wheels' tracks and the vehicle is more stable.

#### 9.4.11. Comparison with the LEGO steering

| Aspect | LEGO steering (previous) | Ackermann steering (current) |
| :--- | :--- | :--- |
| **Geometry** | Parallel: the same angle on both wheels | Ackermann: K = 84 mm, L = 112 mm, arms at 20.56° |
| **Maximum angle** | Small (typical LEGO is 15° to 20°) | 40° inner and 25°–30° outer (measured) |
| **Minimum turning radius** | ≈ 308 to 418 mm at 20° to 15° ($R \approx L / \tan\delta$) | **176 mm**, 43 % smaller than at 20° |
| **Error vs. ideal** | 4.0° at 20° and 8.1° at 30° | 1.1° at 20° and 1.4° at 30° |
| **Tires** | Dragged sideways when turning | Nearly tangent to their turning circles |
| **Pivots** | Press-fit into a chassis cutout, with play | M3 pivot with a defined fit and a lock nut |
| **Stiffness** | LEGO plastic with play | PETG-CF with 0.1 mm clearance per side |
| **Servo range** | — | 98° (−50° to +48°), within the servo's capability |

#### 9.4.12. Design decisions

| Decision | Rejected alternative | Why |
| :--- | :--- | :--- |
| K = 84 mm | K = 96 or 108 mm | Keeps the knuckle lugs out of the rim; almost no steering angle is lost |
| 22 mm steering arm | — | Provides the required tie rod travel with a 14 mm servo horn |
| Servo upside down, in front of the tie rod | Servo behind, with a 25 mm Z-link | Flat link in a single plane: stiffer, with no play from leverage; frees up the center of the chassis |
| Servo resting on the plate | Servo sunk into a recess | The wheel extends 6.85 mm below and the servo is ~28 mm tall: it would hang down to the ground |
| Arms and tie rod above the plate (z 4.5–11) | Tie rod underneath | Avoids rubbing on the ground |
| Gaps filled up to x = ±30 mm | Fully filled | Supports the servo mount and leaves 1 mm against the column and the lug when turning |
| Metal M3 kingpin | Thread turning against the plastic | Prevents plastic wear; the brass insert only holds |
| Ø2.6 mm welding rod with a printed bearing surface | 604ZZ bearings next to the rim | LEGO rims have no room inside, and the load is small (≈ 3 N per wheel) |
| Servo flipped and −50°/+48° range | CAD range (−46.8°/+34.3°) | The actual build needed about 98°; the mechanism analysis confirms it |
| 0.1 mm clearance per side | 0.5 mm clearance | The print does not need that much tolerance; removes the vertical play |
| 25.7 mm adapter and ≈ 143 mm width | 13.7 mm adapter | Matches the front track to the rear track without changing the geometry |
| PETG-CF | Plain PETG | More stiffness: what the PID commands turns into steering without elastic lag |

---

### 9.5. 🦈 Second Level, Standoffs, and the Shark Fin Vision System

The vehicle's second level is the platform that holds the robot's "brain" and its power system. For this version, we redesigned it completely in Onshape with a focus on **Design for Manufacturing (DFM)** and **modularity**, getting rid of improvised fasteners (such as hot glue) and making maintenance in the pits quick and safe.

The main features and structural improvements are:

* **Modular battery compartment:** the battery box is no longer molded into the chassis as one piece. It is now a separate part that attaches to the underside of the platform with 4 screws. The design accepts either brass threaded inserts or long through-bolts with M3 nuts, so it can be replaced without taking the robot apart.
* **Standardized mounts (modular sensor system):** on the first level, we designed raised U-shaped pedestals to fix reading problems with the ToF sensors. That mechanical solution worked so well that we made it a standard for the whole robot. On the front diagonals of this second level, we replicated those built-in pedestals and designed separate mounts for the Sharp sensors.
  * *How it works:* the adapter mount fits into the U pedestal and is fastened rigidly with **two through-bolts and M3 nuts**. The Sharp sensor is then screwed to the adapter.
  * *Advantage:* if we decide to replace the Sharp sensors with more ToF sensors or other hardware in the future, we only need to print a new adapter for the same pedestal, without changing the chassis.
* **Recessed switch housings:** we modeled square pockets with the exact dimensions to mount the physical controls flush, cleanly, and within easy reach from above. On the right (farther forward) is the **main power switch**, and on the left (slightly farther back) is the program's **start button**.
* **PCB assembly guides:** the main perfboard mount and its rear spacer (pillar) are still separate parts attached with instant glue (cyanoacrylate). However, the chassis now has recesses with the exact outline of these parts. They act as a mistake-proof guide (*poka-yoke*) that makes sure the board is perfectly aligned when it is glued.
* **Structured wiring entry:** the second level has two holes with lips on the underside that line up precisely with the holes on the first level. They receive the **cable tubes**, so the wires from the lower sensors run vertically, straight, and protected up to the control board, without tangling with the drivetrain.

<p align="center">
  <img src="other/Base_2_arriba.png" alt="Top view of the second level in CAD" width="700"><br>
  <sub><b>Top view of the second level:</b> showing the square pockets for the flush-mounted front switches and the outlined guides for gluing the perfboard.</sub>
</p>

<br>

<p align="center">
  <img src="other/Adaptadores_Sharp.png" alt="Bottom view of the second level in CAD" width="700"><br>
  <sub><b>Bottom view of the second level:</b> highlighting the new modular battery compartment (in purple) fastened with screws, and the Sharp adapters already installed on their pedestals.</sub>
</p>

#### 9.5.1. Standoffs and threaded inserts

Between the first and second levels are **60 mm standoffs** that we designed ourselves. Ordinary 3D printers cannot make small threads that survive being tightened and loosened many times, so we did not thread the plastic: at each end of the standoff, we heat-set an **M3 brass threaded insert**. This makes the pillars very strong.

#### 9.5.2. Phone mount (adjustable clamp)

The phone is held by two side towers shaped like a **shark fin**. To keep them from giving way under vibration, we fastened them like this:

* **Attachment to the base:** each tower has **3 brass threaded inserts** in its base. The screws go through the second level from below and pull the tower tight against the platform.
* **Design detail (DfAM):** so the inserts heat-set properly, the holes are blind, **4.1 mm in diameter and 9 mm deep**. With those dimensions, the insert goes in and the melted plastic fills its knurling without getting into the thread.

> [!IMPORTANT]
> **Evolution of the Shark Fin system: from a fixed angle to an adjustable one**
>
> - **Version 1 (fixed mount):** the towers held the clamp with a rigid bracket. The phone's tilt could not be changed; any camera adjustment meant taking the part off and modifying it.
> - **Version 2 (adjustable, current):** we removed the rigid bracket and added a screw that goes through each tower from the outside and clamps the base of the fixed jaw. When it is loosened, the base pivots and the camera angle can be recalibrated at the track; when it is tightened, friction locks it in place.
>
> It was a small change, a screw instead of a rigid bracket, but now we can adjust the framing in seconds without special tools or reprinting parts.

The holder itself is an **adjustable clamp**:

1. **Angle:** the two towers hold a central **fixed jaw**, secured by the side screws (which go into their own threaded inserts). Loosening them adjusts the camera angle; tightening them locks it.
2. **Clamping:** a **sliding jaw** moves inside the fixed jaw. On top of the fixed base, we modeled an **M10.5** thread (0.5 mm of clearance to compensate for PETG shrinkage), through which a **3D-printed M10 screw** goes down. Turning it pushes the sliding jaw in a straight line and holds the phone firmly.

<table>
<tr>
<td align="center" width="50%"><img src="other/soporteensamble.png" alt="Shark Fin mount assembly" width="320"><br><sub>Shark Fin mount installed between the towers.</sub></td>
<td align="center" width="50%"><img src="other/vista_tornillo_rosca.png" alt="Printed M10 screw" width="320"><br><sub>M10 screw and M10.5 thread with thermal clearance.</sub></td>
</tr>
</table>

---

### 9.6. 🧩 Full Integration: CAD Master Assembly

All the iterations of Design for Manufacturing (DFM), the standardized components, and the improvements to traction and steering come together in a robust, modular, and highly competitive vehicle.

These final views of the assembly in Onshape show how all the systems work together: the main chassis with its Ackermann steering, the modular rear drivetrain delivering traction without slipping, and the second level carrying the upper electronics with modular sensor mounts.

<p align="center">
  <img src="other/ensamble_diagonal_izq.png" alt="Bottom perspective view of the full assembly" width="800"><br>
  <sub><b>Front-left bottom perspective:</b> detail of the lower Monochassis, showing the front structural base, the Ackermann steering mount, the steering knuckles, and the compact layout of the raised adapters for the ToF sensors.</sub>
</p>

<br>

<p align="center">
  <img src="other/ensamble_diagonal_der.png" alt="Isometric top view of the full assembly" width="800"><br>
  <sub><b>Front-right isometric perspective:</b> overview of the robot's upper packaging, highlighting the shark fin towers with the team logo, the adjustable clamp for the phone mount, and the rear drive block with its built-in gear.</sub>
</p>

---

### 9.7. 🏭 Additive Manufacturing

We printed first on a **Creality K1C** and later on an **Elegoo Centauri Carbon 2**, using **PETG** filament with Orca Slicer and Elegoo Slicer. The steering parts (knuckles, tie rod, link, and servo mount) are printed in **PETG-CF**, which is stiffer. Since the chassis takes constant hits and vibration, we used a profile designed for strength (*DfAM, Design for Additive Manufacturing*):

| Parameter | Value | Reason |
| :--- | :--- | :--- |
| **Nozzle temperature** | 260 °C | Better flow and better *layer adhesion*. |
| **Max. volumetric flow** | 11–12 mm³/s | Limits extruder pressure and prevents under-extrusion. |
| **Infill speed** | 150 mm/s | Prevents wear on the extruder gear during long moves. |
| **Outer perimeters** | 5 walls | Form an outer shell that resists torsion. |
| **Part cooling fan** | 0 % at first / 40 % max. | Overhangs look worse, but the layers do not separate from cooling too fast. |
| **Enclosure** | Open | Keeps heat from rising up the hotend throat (*heat creep*). |

#### 9.7.1. Print optimizations

* **Dowel joints:** parts with large overhangs were split and printed flat. They are joined with 2.5 mm guide dowels with 0.15 mm of clearance (*slip fit*), reinforced with cyanoacrylate.
* **Cooling small parts:** we printed them together with the large ones. The time the print head spends on the other parts gives the small ones time to cool, so they do not deform. We also used a 5 mm *brim* so they stuck well to the bed.

---

### 9.8. 📐 Final Dimensions and Summary of Structural Improvements

| Parameter | Measurement | Status |
| :--- | :--- | :--- |
| **Overall length** | 179 mm | Reduced |
| **Chassis width** | 115 mm | Unchanged |
| **Overall width with wheels** | ≈ 143 mm | Same front and rear |
| **Wheelbase** | 112 mm | ≈ 1:1 with the chassis width |
| **Minimum turning radius** | 176 mm | With the Ackermann steering |

**Mechanical evolution (2025 → 2026):**

* **✓ Maneuverability:** the chassis is shorter, and the Ackermann steering brings the minimum turning radius down to 176 mm.
* **✓ Traction:** a 1.67:1 drivetrain that prioritizes torque, and the reused LEGO wheels with our own adapter.
* **✓ Robustness:** we went from improvised fasteners to a parametric PETG Monochassis with 60 mm standoffs and brass inserts.
* **✓ Vision and center of gravity:** the battery moved down to the lower level, and the Shark Fin system holds the phone with no play.

---

## 10. 🧮 Mass Analysis and Weight Distribution

### 10.1. 📦 Vehicle Mass: 2025 vs. 2026

Moving from the inherited chassis to the **Monochassis** reduced the mass, and we confirmed it on a scale after the final assembly. We need to distinguish **two stages within the 2025 version itself**, because that chassis gained a lot of weight between its first print and its final competition setup:

| Stage | With phone | Without phone |
| :--- | :---: | :---: |
| **v1 (2025): freshly printed chassis** (no tripod or counterweights) | 943 g | 763 g |
| **v1 (2025): final competition setup** (+ tripod + lead counterweights) | **≈1300 g** | ≈1120 g |
| **v2 (2026): current Monochassis** | **1092 g** | **861 g** |

| Comparison | With phone | Without phone |
| :--- | :---: | :---: |
| v2 (2026) vs. v1 competition setup (2025) | 🔻 −208 g (−16.0 %) | 🔻 −259 g (−23.1 %) |

> [!IMPORTANT]
> **What the tripod and counterweights really cost**
>
> The right comparison is not against the 943 g of the first v1 print (which did **not** have the tripod yet), but against its **≈1.3 kg** competition setup. The tripod and, above all, the lead counterweights we added to offset the lever effect (see [section 12](#12--engineering-log-chassis-evolution-v0--v2)) added about **360 g**, almost a third of its mass without the phone. The extra weight of v1 did not come from the chassis itself, but from using weight to correct a geometric flaw (flexing from leverage). v2 solved it at the root with the 1:1 ratio and needs no counterweights.
>
> Compared with the freshly printed 2025 chassis, v2 weighs a bit more (861 g vs. 763 g without the phone), because it now carries four ToF sensors, four regulators, the LED strip, and the Shark Fin mount.

> [!NOTE]
> **Design trade-off: weight vs. stiffness**
>
> We cut up to 259 g from the final 2025 version **without losing stiffness**, even though the Monochassis integrates into a single part the mounts, wiring channels, and housings that used to be separate parts. The reduction comes mainly from removing improvised reinforcements (hot glue, corrective holes, lead counterweights), not from thinning the walls.

### 10.2. 📊 Static Mass Distribution (Front Axle / Rear Axle)

With the phone mounted on the *Shark Fin*, we weighed each axle separately to find the longitudinal center of gravity:

| Axle | Measured mass | Distribution |
| :--- | :---: | :---: |
| **Front** | 660 g | **54.5 %** |
| **Rear** | 550 g | **45.5 %** |
| **Total (measurement setup)** | 1210 g | 100 % |

```mermaid
pie showData
    title Static mass distribution by axle
    "Front axle (54.5 %)" : 660
    "Rear axle (45.5 %)" : 550
```

> [!IMPORTANT]
> **Interpretation**
>
> The front axle carries about 9 points more than a 50/50 split. That is expected: the phone and the *Shark Fin*, the heaviest items mounted up high, sit on the second level toward the front-middle area. The LiPo battery was moved **under the first level** (see [9.5](#95--second-level-standoffs-and-the-shark-fin-vision-system)) precisely to lower the center of gravity, although it does not fully remove the forward bias. This split helps traction when braking and steering response, but it leaves less weight on the drive axle, which we look at in the grip calculation ([11.5](#115--friction-coefficient-and-grip)).

---

## 11. 🚀 Speed, Torque, and Traction

We calculated the drivetrain's linear speed and torque using the vehicle's current mass in its competition setup (with the phone), taking into account the motor's gearbox and our gear drive.

### 11.1. 💨 Vehicle Linear Speed

The 25GA-370 motor turns at a nominal 220 RPM at the output shaft of its gearbox. Converted to radians per second:

$$\omega_{\text{motor}} = \frac{220 \cdot 2\pi}{60} \approx 23.04 \text{ rad/s}$$

The actual diameter of our LEGO wheels (#44309) is 43.2 mm, so the radius is $r = 0.0216$ m. We analyzed the two drivetrain configurations in our design:

* **Configuration A (Torque, 15T to 25T):** reduces speed and multiplies force (1.67:1 ratio).
  $$\omega_{\text{wheel}} = 23.04 \cdot \frac{15}{25} \approx 13.82 \text{ rad/s}$$
  $$v_{\text{max}} = 13.82 \cdot 0.0216 \approx 0.30 \text{ m/s}$$

* **Configuration B (Speed, 20T to 20T):** direct 1:1 drive.
  $$\omega_{\text{wheel}} = \omega_{\text{motor}} \approx 23.04 \text{ rad/s}$$
  $$v_{\text{max}} = 23.04 \cdot 0.0216 \approx 0.50 \text{ m/s}$$

### 11.2. 🔩 Required Torque vs. Available Torque

As an extreme criterion, we calculated the torque needed to hold the vehicle's entire weight at the wheel radius (the equivalent of climbing a vertical wall). On the flat track, the force needed to overcome rolling resistance is minimal:

$$T_{\text{ref}} = m \cdot g \cdot r$$
$$T_{\text{ref}} = 1.092 \text{ kg} \cdot 9.81 \text{ m/s}^2 \cdot 0.0216 \text{ m} \approx 0.231 \text{ N·m}$$

According to the manufacturer, the 25GA-370 motor delivers an output torque of **0.731 N·m**. The torque at the wheels depends on the configuration:

* **Configuration A (Torque, 1.67:1):** $T_{\text{wheel}} = 0.731 \cdot \frac{25}{15} \approx 1.218 \text{ N·m}$
* **Configuration B (Speed, 1:1):** $T_{\text{wheel}} = T_{\text{motor output}} = 0.731 \text{ N·m}$

### 11.3. 🔒 Torque Safety Margin

$$\text{Safety margin} = \frac{T_{\text{wheel}}}{T_{\text{ref}}} \times 100$$

| Configuration | Wheel torque | Safety margin vs. the extreme case (vertical wall) |
| :--- | :---: | :--- |
| **A (15T–25T)** | 1.218 N·m | **527 %** (far more force than a flat track needs) |
| **B (20T–20T)** | 0.731 N·m | **316 %** (ideal balance of traction and speed) |

> [!IMPORTANT]
> **Design flexibility**
>
> The math shows that even with the 1:1 setup (Configuration B), the drivetrain delivers **more than three times (316 %)** the force needed for the worst possible scenario. Configuration A gave us massive torque (527 %) that we were not using. Having both options designed lets us adapt the car: the 1.67:1 ratio for a track with too much friction or unevenness, and 1:1 for speed rounds.

### 11.4. ⏳ Theoretical vs. Actual Lap Time

For the theoretical estimate, we used the ideal center line of the WRO track. With a 3 × 3 m mat and a 1 × 1 m inner square, the center of the lane forms a 2 × 2 m path, which gives a **distance of 8.0 meters per lap**.

| Configuration | Theoretical time (nominal max. speed) | Open lap (measured) | Obstacles (measured) |
| :--- | :---: | :---: | :---: |
| **A (Torque, 1.67:1)** | $\frac{8.0 \text{ m}}{0.30 \text{ m/s}} \approx \mathbf{26.6 \text{ s}}$ | **25.0 s** | **80.0 s** |
| **B (Speed, 1:1)** | $\frac{8.0 \text{ m}}{0.50 \text{ m/s}} \approx \mathbf{16.0 \text{ s}}$ | *[Testing pending]* | *[Testing pending]* |

```mermaid
xychart-beta
    title "Lap time: theoretical vs. actual (8.0 m center path)"
    x-axis ["Theoretical Config A", "Actual Config A (Open Lap)", "Theoretical Config B"]
    y-axis "Time (seconds)" 0 --> 30
    bar [26.6, 25.0, 16.0]
```

> [!NOTE]
> **Analysis: why was the actual time (25.0 s) better than the nominal theoretical time (26.6 s)?**
>
> In practice, the actual time is usually slower than the theoretical one because of friction and corners. However, our car broke that rule in Configuration A. The reason is electrical: the theoretical calculation uses the motor's nominal 220 RPM (which the manufacturer specifies at 6 V). Our power system uses an **XL6009 regulator set up to 8 V**.
>
> By safely overvolting the motor, the actual RPM exceeds the datasheet figure, which makes up for any time lost in the corners. For the new Configuration B (1:1), we expect the same 8 V overvoltage to push the car below the theoretical 16.0 seconds.

### 11.5. 🛞 Friction Coefficient and Grip

To complete the analysis, we need to know how much force the LEGO Technic wheels can transmit to the track mat before they slip:

$$F_{\text{friction}} = \mu \cdot N$$

Where:

- **μ (static friction coefficient, wheel to mat):** we estimate it between **0.6 and 0.8**, a typical range for LEGO rubber tires on textured surfaces similar to the competition mat. We do not have a tribometer, so we use a range instead of a single value.
- **N (normal force on the drive axle):** comes from the measured mass distribution: **550 g × 9.81 m/s² ≈ 5.4 N** on the rear axle.

| Scenario | μ | Max. friction force (rear axle) | Max. acceleration ($F/m$, with 1.092 kg) |
| :--- | :---: | :---: | :---: |
| Conservative | 0.6 | ≈ 3.24 N | ≈ 3.0 m/s² |
| Favorable | 0.8 | ≈ 4.32 N | ≈ 4.0 m/s² |

> [!TIP]
> **What this tells us**
>
> In Configuration A, the motor can push with about **56 N** at the wheel ($1.218 \text{ N·m} / 0.0216 \text{ m}$), far more than the 3.2–4.3 N of grip. This means **acceleration is limited by grip, not by the motor**: if full power is applied all at once, the wheels can spin. Even so, at 3 m/s² the car reaches its top speed (0.50 m/s in Configuration B) in less than 0.2 s, which is more than enough for the track. It also explains why we should not take any more weight off the rear axle.

### 11.6. 🔗 Drivetrain Alternatives Considered: Gears vs. Timing Belt

Before settling on the 1.67:1 gear ratio, we evaluated a timing belt drive. We ruled it out for these reasons:

| Criterion | 🟢 Pinion and spur gear (chosen) | 🔴 Timing belt (rejected) |
| :--- | :--- | :--- |
| **Gear ratio** | Exact and constant (tooth-to-tooth contact) | Can **slip** and change the actual ratio under load |
| **Spare parts in Venezuela** | Readily available: we print them ourselves | Hard to find: belts in specific sizes are difficult to get |
| **Under load** | Stable: the vehicle's mass (861–1092 g) does not affect the mesh | Risk of the belt breaking under the vehicle's weight |
| **Adjustment / tensioning** | Not needed: the mesh requires no tension | Critical: too little tension makes it slip and too much damages the shafts |
| **Maintenance in the pits** | Easy: quick swaps, known tolerances | Difficult: it has to be re-tensioned every time it is touched |
| **Cost of iterating the design** | Low: a new pinion or spur gear is just a reprint | High: depends on finding a custom-size belt |

> [!IMPORTANT]
> **Conclusion**
>
> Gears give a **positive drive**, with no elastic element in between, so the motor's rotation and the wheels' rotation never fall out of sync. That matters because the navigation algorithm, which assumes a known gear ratio, has to behave the same way every time. Also, since we make the parts ourselves, we control the tolerances of every tooth and can change the design in hours instead of depending on suppliers, which is decisive in a country where specific parts (such as custom belts) are hard to get.

---

## 12. 📓 Engineering Log: Chassis Evolution (v0 → v2)

Here we record the failures and redesigns that led to the current Monochassis, following the order diagnosis → root cause → solution.

```mermaid
flowchart TD
    A["🟤 v0: cardboard prototype<br/>(validate the rear drivetrain<br/>and the LEGO steering)"] -->|"Material flexing,<br/>softened areas,<br/>wear from use"| B["🖨️ v1: 3D-printed chassis<br/>(printed by a third party, so<br/>we could not iterate ourselves)"]
    B -->|"Diagnosis: chassis too long<br/>because of the tripod system"| C{"⚠️ Failures found in v1"}
    C --> C1["Flexing in the middle of the chassis<br/>(tripod leverage)"]
    C --> C2["Vertical rear motor:<br/>takes up a lot of space"]
    C --> C3["Non-modular assembly:<br/>hot glue, corrective holes,<br/>hand sanding"]
    C1 --> D["🔧 Temporary fix on v1:<br/>lead counterweights near<br/>the rear axle"]
    C2 --> E["📏 Geometric redesign:<br/>1:1 ratio<br/>(wheelbase = track)"]
    C3 --> F["🔩 Modular redesign:<br/>M3 Allen screws across<br/>the whole platform"]
    D --> G["✅ v2: current Monochassis<br/>(179 × 115 mm, 112 mm wheelbase,<br/>horizontal motor, removable parts)"]
    E --> G
    F --> G
```

### 12.1. 🟤 Iteration v0: Cardboard Prototype

| | |
| :--- | :--- |
| **Goal** | Check that the rear drivetrain and the front steering, both made from LEGO parts, physically fit together before moving to a CAD design. |
| **Method** | Several versions of the cardboard base, adjusting dimensions until both systems fit well. |
| **Failure** | After several tests, the cardboard **bent**, softened in some areas from moisture and use, and showed general fatigue. That was expected from a temporary material. |
| **Root cause** | Cardboard was never chosen as a structural material; it was only there to define dimensions, not to carry loads. |
| **Lesson** | The dimensions validated in cardboard went straight into the CAD for the next version, without repeating trial and error in a material that cannot carry loads. |

### 12.2. 🧪 Iteration v1: First 3D-Printed Chassis

| | |
| :--- | :--- |
| **Goal** | Move the cardboard dimensions to a rigid material and add a tripod to raise the phone. |
| **Failure #1: flexing** | Because it was too long, the chassis visibly bent in the middle. |
| **Root cause #1** | The tripod post acted as a **lever**: it pushed the chassis forward and concentrated the stress in the center of the platform. |
| **Fix #1 (not a root-cause solution)** | We filled pipe caps with lead and placed them near the rear axle as a counterweight. That, plus the tripod itself, raised the mass from 943 g to **≈1300 g** (+357 g): we were correcting a geometric problem with weight instead of addressing the cause. |
| **Failure #2: poor use of space** | The rear drive, although it had a differential, had the motor **vertical** relative to the axle instead of horizontal and parallel. |
| **Root cause #2** | That orientation took up much more volume than needed and took space away from the electronics and wiring. |
| **Failure #3: hard to maintain** | The chassis needed several corrections after printing: extra holes, sanding, and parts glued with hot glue. |
| **Root cause #3** | It was printed by an outside supplier (we did not have our own printer), so we could not correct the CAD and reprint quickly; every error was fixed by hand on the part. |
| **Lesson** | An assembly that depends on glue and manual adjustments cannot be reproduced or repaired in the pits. That is why for v2 we decided to (1) design with the correct tolerances from the CAD stage and (2) use removable M3 Allen screws everywhere. |

### 12.3. ✅ Iteration v2: Monochassis (Current Version, 2026)

| | |
| :--- | :--- |
| **Goal** | Fix all three v1 failures at once: flexing, poor use of space, and difficult maintenance. |
| **Solution to flexing** | A shorter chassis (≈1:1 ratio, 112 mm wheelbase) to eliminate the leverage that bent it. No counterweights are needed anymore. |
| **Solution to space** | The motor sits **horizontal and parallel to the rear axle**, in a recessed cradle built for the 25GA-370 inside the Monochassis. |
| **Solution to maintenance** | The whole platform is assembled with **M3 Allen screws** into heat-set brass inserts. There is no hot glue, and any part comes off without special tools or touch-ups. |
| **Measurable result** | Compared with the v1 competition setup, the mass dropped **259 g without the phone** (1120 g → 861 g) and **208 g with the phone** (≈1300 g → 1092 g), even while integrating more functions into a single part. The reduction came from removing corrective reinforcements, not from thinning the walls. |

---

## 13. 🔧 Step-by-Step Assembly Guide

WRO places a lot of value on a project being **reproducible**. That is why we documented how we built the vehicle, with workshop tips and techniques, so any other team can replicate it. The STL files for every part are in the [`models/`](models/) folder.

```mermaid
flowchart LR
    P1["1. Print the parts"] --> P2["2. Motor and pinion"] --> P3["3. Rear axle and wheels"] --> P4["4. Ackermann steering"]
    P4 --> P5["5. ToF sensors"] --> P6["6. Standoffs"] --> P7["7. Shark Fin towers"] --> P8["8. Second level and electronics"]
```

---

### 13.1. 🔹 Step 1: 3D Printing

First, print all the structural parts in the repository. Our settings are in [section 9.7](#97--additive-manufacturing), but you can adjust them for your printer or the material you use.

> [!TIP]
> **Drivetrain tip:** the gears need as much strength as possible. If you print in PETG, raise the temperature and significantly increase the number of perimeters; that way the teeth will not break under the motor's torque.

<p align="center"><img src="other/ensamble1.jpeg" alt="Printing the parts" width="520"></p>

---

### 13.2. 🔹 Step 2: Motor and Pinion on the Monochassis

With the parts ready, we start with the first level.

1. Fasten the motor to its cradle with M3 screws. **Important:** leave the motor terminals facing up or toward a free side; that makes it much easier to run the wires to the TB6612FNG driver.
2. Fit the pinion onto the motor shaft; it is a *press fit*. The pinion has a rounded side and a small built-in spacer so it stops against the gearbox without rubbing. Check that it is secure.

<p align="center"><img src="other/ensamble2.png" alt="Installing the motor and pinion" width="520"></p>

---

### 13.3. 🔹 Step 3: Rear Axle, Spur Gear, and Wheel Adapters

For the axle, we used an easy-to-find workshop solution: the clean core of a welding electrode.

1. Since the rod is thinner than the bearing bore, build up its diameter with heat-shrink tubing.
2. Put heat-shrink on **one end only**, until it goes into the first bearing in the chassis with some pressure.
3. Pass the rod through that bearing, slide the spur gear (**25T or 20T, depending on the chosen configuration**) onto the middle, and fit the second adapter sleeve on the other end to seat it in the second bearing.
4. Line up the spur gear's teeth with the pinion. Make sure the **set screw** in the spur gear's collar points straight at the *flat spot* you ground in the middle of the rod, and tighten it to lock the gear.
5. Finally, fit the LEGO wheel adapters on both ends of the rod. Line up the **set screws** with the flat spots on the ends and tighten them firmly against the metal.

> [!TIP]
> **To keep it from slipping:** before shrinking the heat-shrink with heat, put a few drops of cyanoacrylate between the metal and the plastic. That way it will not spin on the rod when the motor applies force.

<p align="center"><img src="other/ensamble3.jpeg" alt="Assembling the rear axle" width="520"></p>

---

### 13.4. 🔹 Step 4: Ackermann Steering

The steering parts and dimensions are in [section 9.4](#94--ackermann-steering-system).

![Printed parts of the Ackermann steering](other/componentes_generales_ackerman.jpg)

> [!TIP]
> **Recommended lubrication:** apply **silicone grease** to all moving parts (pivots, bushings, link joints, and knuckle shafts) during assembly. This greatly reduces friction between the printed parts and keeps the steering moving smoothly.

1. **Mounting the steering knuckles:** install the left and right knuckles on their pivots on the chassis. Make sure the Ackermann arm built into each knuckle points diagonally toward the center of the rear axle, as shown in the reference.

   ![Steering knuckles mounted on the chassis](other/paso1_manguetas.jpg)

2. **Preparing the tie rod and link:** connect the link to the tie rod, making sure to fit the matching bushings in each hole so it turns smoothly.

   | 1. Fitting the bushings | 2. Joining the tie rod and link |
   | :---: | :---: |
   | <img src="other/paso2_1_casquillos.jpg.jpg" width="320"> | <img src="other/paso2_2_barra_biela.jpg" width="320"> |

3. **Connecting to the knuckles:** connect both ends of the tie rod to the knuckle arms. **Important:** do not forget to install the bushings in every joint; they are essential to eliminate play and let the mechanism move without binding.

   | 1. Test fit with bushings | 2. Attached to both knuckles |
   | :---: | :---: |
   | <img src="other/paso3_1_presentacion.jpg" width="320"> | <img src="other/paso3_2_union_manguetas.jpg" width="320"> |

4. **Preparing the servo horn:** take the three parts (servo horn, screw, and bushing) and find the hole **14 mm** from the center of the horn.
   * First, drive only the screw through the hole to cut the initial thread.
   * Remove the screw and thread it back in with the bushing on the underside of the horn until it is snug.

   | 1. Horn, screw, and bushing | 2. Cutting the thread at 14 mm | 3. Assembled with the bushing |
   | :---: | :---: | :---: |
   | <img src="other/paso4_1_elementos.jpg" width="220"> | <img src="other/paso4_2_rosca.jpg" width="220"> | <img src="other/paso4_3_ensamblado.jpg" width="220"> |

    > ℹ️ **Note:** depending on the screw's thickness, getting through the horn's plastic may take some force. Make a careful first pass so the screw cuts its own thread, checking that it does not deform or break the servo horn.

5. **Servo mount:** fasten the servo mount in its place on the chassis.

   ![Installing the servo mount](other/paso5_soporte_servo.jpg)

6. **Installing the servo:** mount the servo in its holder, making sure its output shaft faces **right** (seen from the rear of the chassis).

   ![Servo mounted facing right](other/paso6_instalacion_servo.jpg)

7. **Centering and connecting the link:** fit the bushing into the free end of the link and attach it to the servo horn, but **do not screw the horn onto the servo shaft yet**.
   * Power on the system and set the servo electronically to its center (**90°** for a standard 180° servo).
   * Manually align the knuckles so the wheels point perfectly straight ahead and, holding that position, press the horn onto the servo's splined shaft.

   ![Centering and attaching the horn to the servo](other/paso7_calibracion_centro.jpg)

    > ℹ️ **Software fine-tuning:** because of the spline pitch on the servo's output shaft, it is normal for the wheels not to be perfectly straight mechanically once the horn is on. To correct this, do a test run to see which way the vehicle tends to drift and adjust the center-angle *offset* in code until it drives perfectly straight.


8. **Assembling the adapters and wheels:** in each printed adapter, insert a LEGO cross axle into one end and a smooth metal rod (**2.4 mm in diameter × 18 mm long**) into the opposite end. Finally, attach the assembly to the LEGO wheel.

    > ℹ️ **Note:** the smooth rod should press-fit into the adapter. If it feels loose, add a drop of instant glue (cyanoacrylate) so it does not slip or come out while driving.

   | 1. LEGO cross axle | 2. Smooth rod (2.4 × 18 mm) | 3. Attached to the wheel |
   | :---: | :---: | :---: |
   | <img src="other/paso8_1_eje_cruz.jpg" width="220"> | <img src="other/paso8_2_varilla.jpg" width="220"> | <img src="other/paso8_3_rueda.jpg" width="220"> |

9. **Installing the wheels:** slide the smooth rod of the adapter-and-wheel assembly through the center hole of the knuckle.

   ![Installing the wheel on the knuckle](other/paso9_rueda_mangueta.jpg)

10. **Securing with the retainer:** press the round retainer onto the free end of the rod that sticks out on the inner side of the knuckle. Leave a small gap so the wheel turns freely without dragging against the plastic, but without too much end play.

    ![Fitting the retainer on the end of the rod](other/paso10_retenedor.jpg)

---

**Final result of the front end and Ackermann steering:**

![Fully assembled Ackermann steering](other/direccion_ackermann_completa.jpg)

---

### 13.5. 🔹 Step 5: ToF Sensors

Around the edge of the first level (front, rear, left, and right) are the mounts for the VL53L0X laser sensors. Fasten each module to its mount with M3 screws and nuts on the back, so it does not vibrate and the readings are stable.

<p align="center"><img src="other/ensamble5.png" alt="Mounting the ToF sensors" width="520"></p>

---

### 13.6. 🔹 Step 6: Standoffs

Heat-set the M3 brass inserts into each end of the 60 mm standoffs. Once they cool, place them on the first level and fasten them with M3 screws from the underside of the chassis. This completes the first level.

<p align="center"><img src="other/ensamble6.jpeg" alt="Installing the standoffs" width="520"></p>

---

### 13.7. 🔹 Step 7: Shark Fin System (Phone Mount)

Before joining the two levels, prepare the second level with the side towers that hold the phone.

1. Heat-set 3 M3 inserts into the base of each tower (6 in total).
2. Line up the towers with the holes in the second level.
3. Insert the screws **from underneath** the second level so they pull the towers tight against the platform.

*(Because of this bottom-up screwing, we did not attach the second level to the standoffs in the previous step: there would be no room to get the tool in.)*

<p align="center"><img src="other/ensamble7.5.jpeg" alt="Shark Fin mount" width="520"></p>

---

### 13.8. 🔹 Step 8: Final Integration

With the towers firmly in place, set the second level on the standoffs from step 6 and secure it with M3 screws from above. Then mount the adjustable-angle center clamp between the two towers and continue with the wiring and electronics.

<p align="center"><img src="other/ensamble8.jpeg" alt="Final chassis integration" width="520"></p>

---

## 14. 💻 Software and Control

### 14.1. 🧭 Software Continuity and BNO055 Integration

The firmware architecture and the vehicle's overall logic carry over from 2025, because they had already worked well in competition. What changed is where the data comes from.

The biggest change this season is replacing the MPU6050 gyroscope (and the magnetometer we used to correct it) with the **BNO055** IMU. We kept the same orientation logic, but with this sensor the *gyro drift* becomes practically negligible. The effect on the car's behavior is very noticeable:

* **Reliable orientation:** the vehicle always knows its actual angle relative to the starting point.
* **Better lane stability:** it knows precisely when it is straight relative to the corridor, so the servo corrects just enough and does not overcorrect.

### 14.2. 📁 Firmware Files and Libraries

| File | Purpose |
| :--- | :--- |
| [`src/v1PrometeusTeam2026.ino`](src/v1PrometeusTeam2026.ino) | Open lap mode (Open Challenge), 2026 season |
| [`src/v2PrometrusTeam2026.ino`](src/v2PrometrusTeam2026.ino) | Obstacle mode, 2026 season |
| [`src/README.md`](src/README.md) | Instructions for installing the libraries and uploading the code |

**Libraries used in 2026:** `Wire`, `Adafruit_BNO055`, `Adafruit_Sensor`, `Adafruit_VL53L0X`, `Adafruit_TCS34725`, `Adafruit_NeoPixel`, `ESP32Servo`, and `ColorConverterLib`.

### 14.3. 🔄 Shared Navigation Logic

Both programs share this foundation:

* **Startup:** on power-up, the ESP32 initializes the sensors, assigns the ToF addresses, and waits with the LEDs in standby mode until the switch on D23 goes LOW.
* **Orientation reference:** at startup, the BNO055 angle is saved as 0°. At each corner, the target angle advances by 90° (0°, 90°, 180°, 270°).
* **Line detection:** the TCS34725 is read every 30 ms in its own FreeRTOS task. The color is converted to HSV (with `ColorConverterLib`), and a line only counts if it is confirmed over several consecutive readings. After one is counted, the following ones are ignored for 5 s so the same line is not counted twice.
* **Lap direction:** decided by the first line that appears. If it is **blue**, the car turns left at every corner (counterclockwise); if it is **orange**, it turns right (clockwise).
* **End of the run:** four lines make one lap, so after **12 lines** (3 laps) the car drives a short distance and stops.
* **PID turns:** a PID on the angle error (Kp = 2.5, Ki = 0.8, Kd = 0.4) takes the car to the new target angle with a 5° tolerance.
* **PID centering:** a second PID on the distance to the wall (Kp = 2.0, Kd = 1.2) keeps the car on its path. If the angular deviation exceeds 15°, it corrects the angle first and then the distance, so it does not zigzag.
* **Safety:** if a diagonal Sharp sensor detects the wall closer than ~13 cm, the car backs up for a moment and then continues.


**System startup:**

```mermaid
flowchart TD
    A(["Power on"]) --> B["Initialize I2C, servo, motor, and LEDs"]
    B --> C["Turn on the ToF sensors one by one<br/>and assign 0x30 to 0x33"]
    C --> D["Start the BNO055 and save yaw = 0°"]
    D --> E["Create the color sensor task<br/>(every 30 ms)"]
    E --> F{"D23 switch LOW?"}
    F -- No --> F
    F -- Yes --> G(["Start the run"])
```

**Line detection and lap counting:**

```mermaid
flowchart TD
    L1["Read the TCS34725 and convert to HSV"] --> L2{"Blue or orange confirmed<br/>over several readings?"}
    L2 -- No --> L1
    L2 -- Yes --> L3{"5 s since<br/>the last line?"}
    L3 -- No --> L1
    L3 -- Yes --> L4{"First line?"}
    L4 -- Yes --> L5["Set the direction:<br/>blue = left, orange = right"]
    L4 -- No --> L6
    L5 --> L6["Count the line and turn 90°"]
    L6 --> L7{"12 lines?"}
    L7 -- No --> L1
    L7 -- Yes --> L8(["Final stop"])
```

<p align="center">
  <img src="other/diagrama_PID_giro_y_centrado.png" alt="Diagram of the PID turning and centering logic" width="640"><br>
  <sub>PID turning and centering logic.</sub>
</p>

### 14.4. 🏁 Open Lap Mode (Track Without Obstacles)

This algorithm aims to go as fast as possible on the straights and take the corners cleanly. To keep the car aligned, it relies mainly on the IMU angle.

* **Strategy:** the side ToF sensor measures the distance to the guide wall. If the **BNO055** shows that the car has drifted, the system first corrects the angle to stabilize it and then adjusts the distance to the wall.
* **Corners:** the car follows its lane until the color sensor detects a line. At that point, centering is paused and the controlled 90° turn is made. Then it straightens out again and goes back to wall following.


```mermaid
flowchart TD
    A(["Start"]) --> B["Drive forward following the wall<br/>(side ToF + BNO055)"]
    B --> C{"Angular deviation > 15°?"}
    C -- Yes --> D["Correct the angle first"]
    C -- No --> E["Wall-distance PID"]
    D --> F{"Line detected?"}
    E --> F
    F -- No --> B
    F -- Yes --> G["Pause centering and turn 90°<br/>with the angle PID"]
    G --> H{"12 lines?"}
    H -- No --> B
    H -- Yes --> I(["Drive a short distance and stop"])
```

<p align="center">
  <img src="other/Diagrama_codigo_vueltas.png" alt="Diagram of the lap logic" width="640"><br>
  <sub>Lap logic.</sub>
</p>

### 14.5. 🚧 Obstacle Mode: Lane System

**Leaving the parking area.** When the switch is turned on, the car uses the side ToF sensors to measure which side has more room and performs the exit maneuver toward that side.

```mermaid
flowchart LR
    A(["Switch on"]) --> B["Measure left and right ToF"]
    B --> C{"Left < right?"}
    C -- Yes --> D["Exit to the right"]
    C -- No --> E["Exit to the left"]
    D --> F["Wait for the first lane command"]
    E --> F
```

**Lanes.** Instead of avoiding each pillar separately, we split the corridor into lanes: **lane 1**, on the right, for passing red pillars, and **lane 2**, on the left, for green ones. The app recognizes up to two pillars at once and sends a command that describes that combination (for example `C21`, `C15`, or `C33`). That way the car knows in advance which lane to go to and whether it will need to change lanes halfway through the section.

```mermaid
stateDiagram-v2
    [*] --> ESPERANDO_COMANDO_CARRIL
    ESPERANDO_COMANDO_CARRIL --> EVALUANDO_POSICION: command arrives from the app
    EVALUANDO_POSICION --> GIRANDO_HACIA_CARRIL: decides where to move
    GIRANDO_HACIA_CARRIL --> ENDEREZANDO_CARRIL: reaches the lane
    ENDEREZANDO_CARRIL --> MANTENIENDO_CARRIL: angle straight
    MANTENIENDO_CARRIL --> ESTADO_NORMAL: detects a line
    ESTADO_NORMAL --> ESPERANDO_COMANDO_CARRIL: corner turn complete
    MANTENIENDO_CARRIL --> ESTADO_DETENCION_FINAL: line 12
    ESTADO_DETENCION_FINAL --> [*]
```

| State (name in code) | What it does |
| :--- | :--- |
| `ESPERANDO_COMANDO_CARRIL` (waiting for lane command) | Stops the motor and reads the serial port until it receives a valid command from the app. |
| `EVALUANDO_POSICION` (evaluating position) | Measures the side distances to know where it is in the corridor and which way it needs to move. |
| `GIRANDO_HACIA_CARRIL` (turning toward lane) | Turns toward the lane with an angle and duration that depend on the command and the distance to the wall. If the Sharp sensor on that side detects the wall too close, it stops the turn, backs up a little, and continues. |
| `ENDEREZANDO_CARRIL` (straightening in lane) | Counter-steers until it is aligned with the section again (0°, 90°, 180°, or 270°). |
| `MANTENIENDO_CARRIL` (holding lane) | Uses PID to hold the distance to the wall on the lane's side, giving priority to the angle. It keeps this up until the next line. |
| `ESTADO_NORMAL` (normal state) | Makes the corner turn and backs up using the BNO055 angle and the rear ToF as a guide, so it starts the next straight at a known distance from the pillars. |
| `ESTADO_DETENCION_FINAL` (final stop) | After line 12, it drives a short distance and turns off the motors. |

In 2025, after each turn the car backed up "blind" for a fixed amount of time, and we never knew exactly where it would end up. Now that reverse is controlled by the angle and the rear ToF, so every section starts with the car straight and well positioned.

<p align="center">
  <img src="other/diagramaV2.png" alt="Diagram of the lane system logic" width="640"><br>
  <sub>Lane system logic.</sub>
</p>

---

### 14.6. 🧠 Control Architecture and State Machine

An interactive state diagram built for the WRO Future Engineers competition. It lets you simulate the control flows, the ToF sensors, the BNO055 IMU, and the autonomous navigation logic. In addition to the states in the table above, it shows the lateral correction after each turn (`ESTADO_CORRECCION_LATERAL`) and the optimized maneuver used from line 5 onward (`MANIOBRA_OPTIMIZADA`), once the car already knows the track.

**Click the image to open the interactive diagram:**

<p align="center">
  <a href="https://erchivox.github.io/Prometheus-Team-2026/other/ModeloArquitecturaEstados.html"><img src="other/diagrama-de-estados.png" alt="System state diagram" width="900"></a>
</p>

---

---

## 🚧 Optimized Avoidance Cases (Second-Lap Strategy)

With optimized avoidance, our main goal is to **eliminate the time lost crossing over at the corners**, using the information gathered on the first lap about the exact position of the obstacles.

To do this, we implemented a **pattern of guided moves** that decides in advance whether the vehicle should drive in the inner or outer lane, optimizing the path for the combination of pillars detected.

<div align="center">
  <img src="other/casos-de-esquive-optimizados.png" alt="Optimized avoidance cases - Prometheus Team" width="750">
  <br>
  <sub><b>Path illustration:</b> combinations of moves from the exit lane to the arrival lane for each avoidance case.</sub>
</div>

---

### 🧠 How Does the Avoidance Logic Work?

1. **Track memory (first lap):** in the initial phase, the system records the layout of the obstacles detected by the computer vision app, building a decision map for the following sections.
2. **Choosing the maneuver:** depending on whether we are in the **inner (I)** or **outer (E)** lane and where the next section is, the `CommandManager` picks the best transition pattern:
   * **Inner ➔ Inner ($I \to I$):** keeps a tight radius with no abrupt lane changes.
   * **Inner ➔ Outer ($I \to E$):** a controlled opening toward the outer edge.
   * **Outer ➔ Inner ($E \to I$):** closes the path using the free space on the inside.
   * **Outer ➔ Outer ($E \to E$):** smooth travel in the outer lane with a wide radius.
3. **Sensors and gyroscope in a closed loop:** once the maneuver is chosen, the system uses the **ToF distance sensors** and the **BNO055 IMU** to calculate with millimeter precision *when* and *how far* to turn.
4. **Execution and double commands:**
   * In simple maneuvers, the vehicle makes the change and goes back to holding its lane with PID.
   * With **double commands** (two obstacles in a row), the system keeps the navigation pattern active **until the sensors detect that the object has been fully passed**, which is exactly when the change to the other lane is triggered.

> [!NOTE]
> Because it knows the pattern of starting and ending lane commands in advance, the vehicle does not react "blind" to the pillar; it anticipates the turn's kinematics, which greatly reduces lap time in the obstacle round.

## 15. 📱 Telemetry and Computer Vision: Prometheus V5.0 App

Our detection app, built in Android Studio with OpenCV, is now on version 5.0 and serves as the control center of our system during testing. It keeps what already worked in 2025 (USB communication, a full-screen interface, and a built-in serial monitor), but in 2026 we rebuilt much of the image processing and telemetry to handle tougher track conditions.

The app's code lives in its own repository: [APP-WRO-PROMETHEUS-TEAM](https://github.com/SamFlips/APP-WRO-PROMETHEUS-TEAM/tree/master) · [project download as a .zip](https://drive.google.com/file/d/1VvdzdWbxGnchFslUwI8wQfkkwYPTyGRD/view?usp=drive_link).

**Complete Vision and Telemetry Architecture (per frame):**

```mermaid
graph TD
    classDef purple fill:#6a1b9a,stroke:#ab47bc,stroke-width:2px,color:#fff;
    classDef cyan fill:#0277bd,stroke:#29b6f6,stroke-width:2px,color:#fff;
    classDef green fill:#00695c,stroke:#26a69a,stroke-width:2px,color:#fff;
    classDef dark fill:#1a237e,stroke:#448aff,stroke-width:2px,color:#fff;

    A["CameraX API<br>720 x 1280<br>YUV Frame"]:::purple --> B["ColorAnalyzer<br>80% Centered<br>ROI Crop"]:::cyan
    B --> C["HSV Space<br>CLAHE on V<br>Channel"]:::cyan
    C --> D["Color Masks<br>Red, Green,<br>Magenta, Wall"]:::cyan
    D --> E["Morphology<br>OPEN 7x7 and<br>CLOSE 3x3"]:::cyan
    E --> F["Shape Filter<br>Solidity, Hull,<br>Internal Sat"]:::green
    F --> G["Quality Score<br>60% Solidity<br>40% Sat"]:::green
    G --> H["Spatial Model<br>Distance cm<br>60% L/R Split"]:::green
    H --> I["CommandManager<br>Single, Dual,<br>Parking Cases"]:::purple
    I --> J["Time Filter<br>8 Frames, 70%<br>Votes, 2s Hold"]:::purple
    J --> K["USB Serial<br>115200 bps to<br>ESP32 MCU"]:::dark
```

---

### 15.1. 📺 V5.0 Interface and Built-In Serial Monitor

| <img src="other/app2026.jpeg" width="400" alt="V5.0 main interface"> | <img src="other/Monitor_serial.jpeg" width="400" alt="Built-in serial monitor"> |
| :---: | :---: |
| *Main interface with two-object detection and stabilization.* | *Built-in serial monitor running in real time.* |

These are the improvements we made to the core of the app (`MainActivity.kt`, `ColorAnalyzer.kt`, and the settings system):

---

### 15.2. 🎯 On-Track Calibration and Dynamic Menu

We no longer need to open Android Studio to adjust variables at the competition venue. Through `SettingsDialogManager.kt`, all critical parameters can be tuned live on the track and exported/imported as JSON files:

* **8-Sample On-Track HSV Calibration:** Ambient lighting varies across the four sides of the $3 \times 3\text{ m}$ mat. The app includes a translucent bottom-half calibration dialog that activates `ColorAnalyzer.calibrationMode = true` (widening HSV ranges to $S, V \ge 10$ to capture even severely backlit pillars). The operator takes **8 samples** per color (4 in the standard direction, Tramos 1–4, and 4 in the reverse direction, Tramos 5–8). The app computes the mean Hue ($\bar{H}$) and tolerance from the internal pixels of the contour and updates the active masks immediately.
* **Automatic Red Band-2 Clipping:** Because Red wraps around the HSV cylinder ($0^\circ\text{–}15^\circ$ and $160^\circ\text{–}180^\circ$), its upper band overlaps with Magenta ($145^\circ\text{–}170^\circ$). When Magenta is calibrated on-track, `ColorConfig` automatically clips the lower limit of Red Band 2 so the parking marker never leaks into the red pillar mask.
* **Live Parameter & Hardware Tuning:** Distance zones (Front, Middle, Back), reference area/distance, internal saturation thresholds (0–255 separately for Red and Green), phone flashlight (torch) state, Black Wall detection, and Parking Mode can all be toggled in real time.

---

### 15.3. ✨ Anti-Glare Filter and Shape Cleanup

To deal with lighting changes, shadows, and floor reflections on the vinyl mat, `ColorAnalyzer.kt` applies a multi-stage filtering pipeline before accepting any contour:

1. **CLAHE Illumination Normalization:** The RGB frame inside the 80% centered ROI is converted to HSV, its channels are split, and **CLAHE** (*Contrast Limited Adaptive Histogram Equalization*, `clipLimit = 1.2`, `tileGridSize = 8x8`) is applied exclusively to the **V (brightness) channel** before recombining. This stabilizes exposure under spotlights without distorting Hue or Saturation.
2. **Asymmetric Morphological Cleanup:** Each binary color mask undergoes an **OPEN** operation with a large $7 \times 7$ rectangular kernel to break thin specular reflection bridges connecting a pillar to floor glare, followed by a **CLOSE** operation with a smaller $3 \times 3$ kernel to fill internal pinholes without re-joining separated blobs.
3. **Double-Pass Polygonal Smoothing:** Contours are smoothed via `approxPolyDP` in two passes: an aggressive pass ($\epsilon_1 = 0.008 \times \text{perimeter}$) to strip pixelation noise, and a fine pass ($\epsilon_2 = 0.004 \times \text{perimeter}$) to preserve structural corners.
4. **Geometric Validation (Pillar vs. Magenta Profiles):** Because pillars are tall vertical prisms while the magenta parking marker is a flat horizontal plate, the app enforces two distinct sets of geometric thresholds:

| Geometric Metric | Formula / Method | Red & Green Pillars | Magenta Parking Marker | Purpose |
| :--- | :--- | :---: | :---: | :--- |
| **Minimum Area** | `Imgproc.contourArea` | $> 280\text{ px}^2$ | $> 840\text{ px}^2$ | Rejects distant background noise |
| **Solidity** | $\text{Area} / \text{Convex Hull Area}$ | $\ge 0.40$ | $\ge 0.55$ | Rejects fragmented or hollow blobs |
| **Aspect Ratio** | $\text{Width} / \text{Height}$ | $0.25 \text{ to } 1.20$ | $0.15 \text{ to } 8.00$ | Pillars $> 1.20$ are reclassified as Parking |
| **Rectangularity (Extent)** | $\text{Area} / (\text{Width} \times \text{Height})$ | $\ge 0.35$ | $\ge 0.35$ | Ensures the contour fills its bounding box |
| **Complexity Ratio** | $\text{Perimeter}^2 / \text{Area}$ | $\le 30.0$ | $\le 45.0$ | Rejects jagged, irregular shapes |
| **Convexity** | $\text{Area} / \text{Hull Area}$ (strict) | $\ge 0.85$ | $\ge 0.65$ | Rejects shapes with deep indentations |
| **Convexity Defects** | `Imgproc.convexityDefects` | $\le 3$ | $\le 6$ | Counts concavities along the perimeter |
| **Polygon Vertices** | `approxPolyDP` ($0.02 \times P$) | $4 \text{ to } 12$ | *Exempt* | Ensures prism-like geometry |
| **Sharp Spike Filter** | Dot-product vertex angle | $\le 2$ angles $< 45^\circ$ | *Exempt* | Rejects triangular glare spikes |

5. **Internal Saturation Anti-Glare Filter (`hasRealObjectSaturation`):** Floor reflections wash out color by mixing it with the white/gray mat. The app rasterizes a binary mask of the contour's interior and computes the mean of the **S (Saturation)** channel exclusively inside the object. If the average saturation falls below the configured threshold (`minInternalSaturationRed` or `minInternalSaturationGreen`), the contour is discarded as glare and logged on-screen with its rejection reason (`✗ reflejo (sat. baja)`).
6. **Candidate Quality Score:** Valid contours within the track's maximum distance (`backDistanceMax`) are ranked by distance and a composite quality score:

$$Q_{\text{score}} = 0.60 \cdot \left(\frac{\text{Area}}{\text{Width} \times \text{Height}}\right) + 0.40 \cdot \left(\frac{\bar{S}_{\text{internal}}}{255}\right)$$

---

### 15.4. 🟣 Parking Mode (Magenta Zone)

Under WRO Future Engineers rules, the presence of the magenta parking lot forces all traffic signs in that section to shift toward the inner wall. Our app does not only use magenta to park at the end of the run; it uses it to **dynamically switch the avoidance command tables (`C...E`)** during all three laps so the car never collides with the parking barriers.

#### Multi-Signal Parking Detection (Redundancy)
Lighting or oblique camera angles can make the magenta barrier look dark or shift its apparent Hue toward red. To guarantee detection, `ColorAnalyzer` activates Parking Mode through **two independent physical triggers** (plus two geometric reclassifications):
1. **Magenta Color Channel (with Area Gate):** A valid magenta contour must exceed `minMagentaActivationArea = 8000 px²`. Seeing the marker from the previous straight produces a small area ($840\text{–}7999\text{ px}^2$) which is ignored (`🟣⚠️ Magenta LEJOS`); the mode only activates when the vehicle is actually inside the parking section.
2. **Wide Red Reclassification (`aspectRatio > 1.2`):** Before applying the strict pillar filter, if a red contour has a width-to-height ratio $> 1.2$, it is physically too wide to be a vertical pillar (which is taller than it is wide) and is automatically reclassified as the horizontal parking barrier.
3. **Max Plausible Area Reclassification:** Using the minimum possible front distance (`frontDistanceMin`), the app computes the maximum pixel area a real pillar can ever have (`maxPlausibleRedGreenArea`). Any red contour exceeding this physical limit is reclassified as Magenta.
4. **Hue-Independent Black Wall Detector (`maskWall`):** Because the outer perimeter wall is black ($V \le 70$, $S \le 90$), the app searches for large dark rectangular regions ($\text{Area} \ge 5000\text{ px}^2$, $\text{Rectangularity} \ge 0.55$) that touch the bottom of the ROI (`touchesBottom`) and either the left (`touchesLeft`) or right (`touchesRight`) edge of the frame within a 5% margin.

#### Chassis Rotation Immunity (Orientation by Deduction)
When the vehicle enters a section right after a 90° corner turn, the chassis may still be slightly angled. Relying on the screen's horizontal center line to tell whether a pillar is on the Left (`L`) or Right (`R`) would fail if the camera's yaw shifts the pillar across the dividing line.

To solve this, `CommandManager` implements **Orientation by Deduction**:
* The moment Parking Mode activates, the app locks the side where the parking barrier/wall sits (`parkingSide = "L"` or `"R"`).
* Because WRO rules mandate that pillars in the parking section are placed against the **opposite** (inner) wall, the app ignores the pillar's X coordinate on screen and forces its effective orientation to the opposite side (`parkingSide == "L" → effectiveOrientation = "R"`, and vice versa).

```mermaid
graph TD
    classDef purple fill:#6a1b9a,stroke:#ab47bc,stroke-width:2px,color:#fff;
    classDef cyan fill:#0277bd,stroke:#29b6f6,stroke-width:2px,color:#fff;
    classDef green fill:#00695c,stroke:#26a69a,stroke-width:2px,color:#fff;

    A["Frame Analysis<br>ColorAnalyzer"]:::purple --> B{"Magenta<br>8000 px or<br>Black Wall?"}:::cyan
    B -- "No" --> C["Standard Mode<br>Normal 60%<br>L / R Split"]:::green
    B -- "Yes" --> D["Parking Mode<br>Lock Side<br>L or R"]:::purple
    D --> E{"Which side<br>is locked in<br>parkingSide?"}:::cyan
    E -- "Left (L)" --> F["Force Right R<br>Use Table R<br>C01E to C30E"]:::green
    E -- "Right (R)" --> G["Force Left L<br>Use Table L<br>C07E to C38E"]:::green
```

---

### 15.5. ⏳ Time-Based Stabilization Filter

Sending raw frame-by-frame detections over serial would flood the ESP32 and trigger erratic steering maneuvers from single-frame motion blur. `MainActivity.kt` implements a dedicated thread (`temporalFilterExecutor`) running a **Sliding-Window Majority Voting Filter**:

* **Sliding Window (`DETECTION_WINDOW_SIZE = 8`):** Every valid frame pushes its resulting command into an 8-frame FIFO queue (`ArrayDeque`). If the time gap between two consecutive frames exceeds `MAX_FRAME_GAP_MS = 300 ms`, the window and timers reset automatically.
* **Majority Ratio (`DETECTION_MAJORITY_RATIO = 0.70`):** The filter tallies command frequencies inside the window. A candidate command must hold at least **70% of the votes** (at least 6 out of 8 frames) to be considered dominant.
* **Stability Hold Timer (`DETECTION_STABILITY_TIME = 2000 ms`):** Once a dominant command emerges, it must remain the uninterrupted winner for **2000 ms** before `sendCommandImmediate()` dispatches it to the ESP32 and triggers the unique confirmation audio tone (`AudioManager.playCommandSoundOnce`).
* **Expiry & Re-dispatch (`STABLE_STATE_EXPIRY_TIME = 3000 ms`):** After a stable command is sent, further transmissions are locked for 3 seconds to prevent serial saturation, after which the state resets to allow re-confirmation if the robot is still awaiting instructions.
* **Instant Clear on Timeout (`DETECTION_TIMEOUT = 300 ms`):** If no valid object is seen for 300 ms, `startDetectionMonitor()` bypasses the 2-second hold, clears the voting window, and immediately dispatches `"N"` (No Detection).

```mermaid
graph TD
    classDef purple fill:#6a1b9a,stroke:#ab47bc,stroke-width:2px,color:#fff;
    classDef cyan fill:#0277bd,stroke:#29b6f6,stroke-width:2px,color:#fff;
    classDef green fill:#00695c,stroke:#26a69a,stroke-width:2px,color:#fff;
    classDef dark fill:#1a237e,stroke:#448aff,stroke-width:2px,color:#fff;

    A["New Command<br>from Frame"]:::purple --> B{"Is Command<br>equal to<br>N?"}:::cyan
    B -- "Yes" --> C["Clear Window<br>Send N<br>Immediately"]:::dark
    B -- "No" --> D["Check Gap<br>Reset if<br>over 300 ms"]:::cyan
    D --> E["Sliding Window<br>Push Frame<br>Max 8 FIFO"]:::cyan
    E --> F{"Window 8/8<br>and Votes<br>over 70%?"}:::cyan
    F -- "No" --> G["Wait Next<br>Frame and<br>Update UI"]:::purple
    F -- "Yes" --> H{"Held Stable<br>for at least<br>2000 ms?"}:::green
    H -- "No" --> G
    H -- "Yes" --> I["Mark STABLE<br>Send USB and<br>Audio Tone"]:::dark
```

---

### 15.6. 👀 Dual Detection and Case-Based Architecture

Building on the two-object detection we added at the end of 2025, that information now feeds a more complete decision system:

* The app still gets the area, distance, and position of a primary and a secondary object at the same time. Distance is estimated from the pillar's apparent area, calibrated once at a known distance: $d = d_{\text{ref}} \sqrt{A_{\text{ref}} / A_{\text{measured}}}$.
* All the decision logic was moved off the main thread into a separate `CommandManager`, which analyzes the information for both objects against a list of cases and sends the matching lane command (see [14.5](#145--obstacle-mode-lane-system)).
* To handle more data with the ESP32, we improved the USB communication: the app discards the binary garbage and *boot logs* the ESP32 sends when it connects, so readings are clean from the very first moment.

> [!TIP]
> **Convenience during testing (carried over from 2025)**
>
> The interface runs full screen, wakes the phone when the USB cable is plugged in, and plays a different tone for each command sent. That way the team can follow what the car is doing without taking their eyes off it.

<p align="center">
  <img src="other/Diagrama_de_flujo_app.png" alt="App flowchart" width="640"><br>
  <sub>App flowchart.</sub>
</p>

**Video of the app in action** (click the image):

<p align="center">
  <a href="https://youtu.be/TpLVCiggEwY"><img src="other/Funcionamiento-en-vivo.png" alt="Video of the app running live" width="520"></a>
</p>

---

### 15.7. 📐 Mathematical and Spatial Model

To convert 2D pixel contours into 3D navigation decisions, `CommandManager.kt` and `ColorAnalyzer.kt` share a unified mathematical model:

#### 1. Area-Based Distance Estimation
According to perspective projection, the apparent area of an object is inversely proportional to the square of its distance from the camera lens. Calibrating a known reference area ($A_{\text{ref}}$, default $19000\text{ px}^2$) at a known reference distance ($d_{\text{ref}}$, default $50\text{ cm}$), the raw distance $d_{\text{raw}}$ (in cm) is computed as:

$$d_{\text{raw}} = d_{\text{ref}} \cdot \sqrt{\frac{A_{\text{ref}}}{A_{\text{measured}}}}$$

Contours with $d_{\text{raw}} > d_{\text{back,max}}$ are discarded so obstacles in distant corridors are ignored. For valid objects, the operational distance is clamped to the system range:

$$d_{\text{final}} = \min\left(\max\left(\lfloor d_{\text{raw}} \rfloor, \, 10\text{ cm}\right), \, 200\text{ cm}\right)$$

#### 2. Distance Zones
Each detected object is classified into one of three longitudinal zones along the corridor (default calibrated ranges shown; fully adjustable in the UI):
* **Front Zone (`Adelante`):** $50\text{ cm} \le d \le 80\text{ cm}$
* **Middle Zone (`Medio`):** $100\text{ cm} \le d \le 130\text{ cm}$
* **Back Zone (`Atrás`):** $140\text{ cm} \le d \le 180\text{ cm}$

#### 3. Asymmetric 60% Spatial Split (Left / Right Orientation)
Because the smartphone camera is rotated $90^\circ$ in software to match its vertical mounting and has a slight lateral offset relative to the chassis centerline, a 50/50 split would misclassify center-lane pillars during approaches. We define the normalized horizontal centroid $X_{\text{norm}}$ and apply a **60% threshold**:

$$X_{\text{norm}} = \frac{X_{\text{center}}}{W_{\text{ROI}}} \quad \Longrightarrow \quad \text{Orientation} = \begin{cases} \text{L (Left)} & \text{if } X_{\text{norm}} < 0.60 \\ \text{R (Right)} & \text{if } X_{\text{norm}} \ge 0.60 \end{cases}$$

*(In the live camera preview, this threshold is rendered as a horizontal yellow dividing line across the rotated frame at $60\%$ of its height).*

---

### 15.8. 📋 Complete Command Matrix (CommandManager)

Instead of streaming raw coordinates, the app evaluates up to two simultaneous pillars—**Primary** (closest) and **Secondary** (second closest)—and dispatches a single compact case code to the ESP32 state machine:

#### A. Single-Object Cases (Standard Mode vs. Parking Mode `E`)
When only one pillar is visible in the corridor, the command depends on its color, longitudinal zone, and lateral orientation (`L` or `R`). In Parking Mode (`E`), only the side opposite to `parkingSide` is valid:

| Zone | Distance Range (default) | Color | Orientation | Standard Command | Parking Command (`E`) | Description |
| :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| **Front** | $50\text{–}80\text{ cm}$ | 🟢 Green | Right (`R`) | **`C01`** | **`C01E`** | Green, Front Right |
| **Front** | $50\text{–}80\text{ cm}$ | 🔴 Red | Right (`R`) | **`C02`** | **`C02E`** | Red, Front Right |
| **Front** | $50\text{–}80\text{ cm}$ | 🟢 Green | Left (`L`) | **`C07`** | **`C07E`** | Green, Front Left |
| **Front** | $50\text{–}80\text{ cm}$ | 🔴 Red | Left (`L`) | **`C08`** | **`C08E`** | Red, Front Left |
| **Middle** | $100\text{–}130\text{ cm}$ | 🟢 Green | Right (`R`) | **`C03`** | **`C03E`** | Green, Middle Right |
| **Middle** | $100\text{–}130\text{ cm}$ | 🔴 Red | Right (`R`) | **`C04`** | **`C04E`** | Red, Middle Right |
| **Middle** | $100\text{–}130\text{ cm}$ | 🟢 Green | Left (`L`) | **`C09`** | **`C09E`** | Green, Middle Left |
| **Middle** | $100\text{–}130\text{ cm}$ | 🔴 Red | Left (`L`) | **`C10`** | **`C10E`** | Red, Middle Left |
| **Back** | $140\text{–}180\text{ cm}$ | 🟢 Green | Right (`R`) | **`C05`** | **`C05E`** | Green, Back Right |
| **Back** | $140\text{–}180\text{ cm}$ | 🔴 Red | Right (`R`) | **`C06`** | **`C06E`** | Red, Back Right |
| **Back** | $140\text{–}180\text{ cm}$ | 🟢 Green | Left (`L`) | **`C37`** | **`C37E`** | Green, Back Left |
| **Back** | $140\text{–}180\text{ cm}$ | 🔴 Red | Left (`L`) | **`C38`** | **`C38E`** | Red, Back Left |

#### B. Dual-Object Cases (Primary in Front Zone + Secondary in Back Zone)
For a dual case to trigger, `isValidDualConfiguration()` verifies that the **Primary** object is in the **Front Zone** and the **Secondary** object is in the **Back Zone** (otherwise, it safely falls back to the Primary object's single command). In Parking Mode (`E`), since all pillars are shifted against the inner wall, only same-side combinations (`L+L` or `R+R`) exist:

| Pattern | Primary (Front Zone) | Secondary (Back Zone) | Standard Command | Parking Command (`E`) | Maneuver Summary |
| :---: | :---: | :---: | :---: | :---: | :--- |
| **L ➔ R** | 🟢 Green (`L`) | 🟢 Green (`R`) | **`C13`** | *Falls back to single* | Green Front Left + Green Back Right |
| **L ➔ R** | 🟢 Green (`L`) | 🔴 Red (`R`) | **`C14`** | *Falls back to single* | Green Front Left + Red Back Right |
| **L ➔ R** | 🔴 Red (`L`) | 🟢 Green (`R`) | **`C15`** | *Falls back to single* | Red Front Left + Green Back Right |
| **L ➔ R** | 🔴 Red (`L`) | 🔴 Red (`R`) | **`C18`** | *Falls back to single* | Red Front Left + Red Back Right |
| **R ➔ L** | 🟢 Green (`R`) | 🟢 Green (`L`) | **`C19`** | *Falls back to single* | Green Front Right + Green Back Left |
| **R ➔ L** | 🟢 Green (`R`) | 🔴 Red (`L`) | **`C20`** | *Falls back to single* | Green Front Right + Red Back Left |
| **R ➔ L** | 🔴 Red (`R`) | 🟢 Green (`L`) | **`C21`** | *Falls back to single* | Red Front Right + Green Back Left |
| **R ➔ L** | 🔴 Red (`R`) | 🔴 Red (`L`) | **`C24`** | *Falls back to single* | Red Front Right + Red Back Left |
| **R ➔ R** | 🟢 Green (`R`) | 🟢 Green (`R`) | **`C25`** | **`C25E`** | Green Front Right + Green Back Right |
| **R ➔ R** | 🟢 Green (`R`) | 🔴 Red (`R`) | **`C26`** | **`C26E`** | Green Front Right + Red Back Right |
| **R ➔ R** | 🔴 Red (`R`) | 🟢 Green (`R`) | **`C27`** | **`C27E`** | Red Front Right + Green Back Right |
| **R ➔ R** | 🔴 Red (`R`) | 🔴 Red (`R`) | **`C30`** | **`C30E`** | Red Front Right + Red Back Right |
| **L ➔ L** | 🟢 Green (`L`) | 🟢 Green (`L`) | **`C31`** | **`C31E`** | Green Front Left + Green Back Left |
| **L ➔ L** | 🟢 Green (`L`) | 🔴 Red (`L`) | **`C32`** | **`C32E`** | Green Front Left + Red Back Left |
| **L ➔ L** | 🔴 Red (`L`) | 🟢 Green (`L`) | **`C33`** | **`C33E`** | Red Front Left + Green Back Left |
| **L ➔ L** | 🔴 Red (`L`) | 🔴 Red (`L`) | **`C36`** | **`C36E`** | Red Front Left + Red Back Left |

---

### 15.9. 🔌 USB-Serial Protocol and Fault Tolerance

Reliable communication between Android and the ESP32 over an OTG cable subject to mechanical vibration required dedicated hardware/software safeguards in `MainActivity.kt` and `SerialMonitorActivity.kt`:

* **Custom VID/PID Probing (CH9102X Fallback):** In addition to standard CP210x (`0x10C4:0xEA60`), FTDI (`0x0403:0x6001`), CH340 (`0x1A86:0x7523`), and native ESP32-S2/S3/C3 USB IDs, the app registers a custom `ProbeTable` entry for the **WCH CH9102X** chip (`0x1A86:0x55D4`) mapped to the `Ch34xSerialDriver`, ensuring plug-and-play compatibility across different ESP32 DevKit revisions.
* **Controlled DTR/RTS Reset & Boot-Garbage Purge:** On connection (`115200 bps, 8N1`), `initializeESP32()` executes a timed DTR/RTS handshake (`DTR=false, RTS=true` $\to 100\text{ ms} \to$ `RTS=false` $\to 50\text{ ms} \to$ `DTR=true`), purges the hardware buffers 3 times, and runs `clearInitialDeviceData()`. This function actively reads and discards incoming bytes for up to $1500\text{ ms}$ (or exits early after 3 consecutive empty reads $\approx 600\text{ ms}$) to flush ESP32 ROM bootloaders (`rst:0x...`, `boot:0x...`, `ets_main.c`, `0x00`/`0xFF` bytes) so they never corrupt the telemetry parser.
* **Multi-Threaded Priority Architecture & Watchdogs:**
  * `CameraThread`, `USBThread`, and `CommandProcessor` run on dedicated single-thread executors with `Thread.MAX_PRIORITY`.
  * **Camera Watchdog (`startDetectionHealthMonitor`):** Every 5 seconds, a coroutine verifies that `isDetectionActive` was set by `ColorAnalyzer`. If the camera pipeline stalls while USB is connected, it automatically re-binds CameraX.
  * **Auto-Wake & Auto-Reconnect:** Plugging in the OTG cable triggers `ACTION_USB_DEVICE_ATTACHED`, which acquires a `FULL_WAKE_LOCK` (`ACQUIRE_CAUSES_WAKEUP`) to turn on the screen over the lockscreen and connects automatically after a 2-second stabilization delay. If a write fails after `MAX_WRITE_RETRIES = 2`, `reconnectDevice()` safely closes the port and re-initializes the link after $1200\text{ ms}$.
---

## 16. 🎬 Track Tests and Videos

For testing, we had a track printed on vinyl that is identical to the competition one: 3 × 3 meters, with the same dimensions, colors, and corner lines. That way we test the vehicle on the same surface and with the same colors it will face at the competition, and the adjustments we make at home (color sensor, camera, and control) carry over directly on the day of the round.

Links to all the videos are also in [`video/video.md`](video/video.md).

Click the images to watch the videos:

<table>
<tr>
<td align="center" width="50%"><b>Track lap test</b><br><br><a href="https://youtube.com/shorts/Ml_8qpX_Ozo"><img src="other/videos.jpeg" alt="2026 track lap video" width="360"></a></td>
<td align="center" width="50%"><b>Obstacle avoidance test</b><br><br><a href="https://youtu.be/qwc4M6uargw?si=8OC0NmMWCrHWz1Wq"><img src="other/videos.jpeg" alt="2026 obstacle avoidance video" width="360"></a></td>
</tr>
</table>

---

## 17. 🚀 Upcoming Improvements

These are the hardware changes we have already decided on for the next version of the vehicle. They are not installed yet: once we test them on the track, we will update the results in this README. The new components already appear, marked with 🔜, in the tables in sections [4](#4--control-sensing-and-interface-components) and [5](#5--power-and-actuation-components).

| # | Change | Current | New | Expected improvement |
| :---: | :--- | :--- | :--- | :--- |
| 1 | **Front and side ToF** | 3 × VL53L0X (up to 2 m) | 3 × **VL53L1X** (up to 4 m, 27° field of view with a programmable region of interest) | Longer range and more stable readings when following the wall and approaching each corner |
| 2 | **Rear ToF** | 1 × VL53L0X (a single measurement) | 1 × **VL53L5CX** multizone (4×4 or 8×8 zones, 45° × 45°, up to 4 m) | A depth "image" of what is behind: more precise guided reversing |
| 3 | **Diagonal sensors** | 2 × Sharp GP2Y0A21 (analog, 10–80 cm) | 2 × **VL53L0X** (the ones freed up by changes 1 and 2) | Digital I2C measurements, with no analog noise, using the same type of sensor as the rest |
| 4 | **Steering servo** | MG90 (analog) | **EMAX ES08MD** (digital) | More precise position control and better angle holding |

### 17.1. 📡 New Sensor Layout

```mermaid
flowchart TB
    F["🎯 Front<br/>VL53L1X"]
    DI["↖️ Left diagonal<br/>VL53L0X"]
    DD["↗️ Right diagonal<br/>VL53L0X"]
    I["◀️ Left side<br/>VL53L1X"]
    CAR["🚗 Vehicle<br/>ESP32 + BNO055"]
    D["▶️ Right side<br/>VL53L1X"]
    T["🔲 Rear<br/>VL53L5CX (8×8 zones)"]

    DI --- F --- DD
    I --- CAR --- D
    F --- CAR
    CAR --- T
```

With this change, the vehicle will have **six ToF sensors on I2C bus #1** and no analog distance sensors.

### 17.2. ✅ What We Expect to Gain from Each Change

- **VL53L1X at the front and sides.** Doubles the VL53L0X's range (from 2 m to 4 m) and lets us choose the distance mode and timing budget for each situation: fast and short near the wall, long when approaching a corner. Its programmable region of interest lets us aim the sensor at part of its field of view, for example to avoid "seeing" the floor.
- **VL53L5CX at the rear.** Instead of a single distance, it returns a grid of up to 8 × 8 distances. When the car reverses after each turn (section 14.5), this tells it whether the wall behind is straight or angled, not just how far away it is. It also helps detect the edges of the parking zone at the end of the obstacle challenge.
- **VL53L0X on the diagonals.** The Sharp sensors are analog: their readings depend on the ESP32's conversion and on power supply noise. A digital ToF sensor returns the distance directly in millimeters. We also reuse the VL53L0X sensors coming off the front, sides, and rear.
- **EMAX ES08MD digital servo.** A digital servo corrects its position many more times per second than an analog one, so it holds the angle better when the wheels push back and has a smaller dead band. It delivers **2.0 kgf·cm at 6 V**: that is the value we already use in the steering torque calculation (9.4.6), which leaves a margin of more than 6×. It is the servo we had planned for the Ackermann steering (9.4.1).

### 17.3. 🔧 What We Need to Adjust to Integrate Them

| Change | Required adjustment |
| :--- | :--- |
| **Six ToF sensors on the same bus** | Assign six different I2C addresses at power-up, as we do today with four (section 7.2.2). The two diagonal ToF sensors need two more XSHUT pins; they must be output pins, because GPIO 34 and 35, freed up by the Sharp sensors, are input-only. |
| **VL53L5CX** | Uses its own library ([SparkFun VL53L5CX](https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library)) and loads internal firmware at startup, so booting takes a bit longer. Its address is also changed in software, but it is enabled with the LPn pin instead of XSHUT. |
| **VL53L1X** | New library ([Pololu VL53L1X](https://github.com/pololu/vl53l1x-arduino)) and choosing the distance mode and timing budget. |
| **Power draw** | The VL53L5CX draws about 100 mA while ranging (up to 150 mA at peaks) and each VL53L1X about 20 mA, compared with 15–20 mA for each VL53L0X. We need to check the ESP32's 3.3 V line and update the power and battery life calculation (section 8). |
| **Firmware** | Replace the analog Sharp readings with the diagonal ToF readings in the safety logic (emergency reverse) and recalibrate their thresholds. |
| **EMAX servo** | Recalibrate the µs per degree and check the steering travel (currently −50° to +48°), leaving a margin from the end stops. |

---

## 18. 🌐 Use of GitHub and Artificial Intelligence

### 18.1. 🌿 Workflow

- **`main` is the stable version.** Major changes are made on a separate branch and merged into `main` through a *pull request*, which the team reviews before merging.
- **Full history:** every change is recorded in the [commit history](https://github.com/erchivox/Prometheus-Team-2026/commits/main), from the 2025 versions to the current one.
- **Standard WRO folders:** `t-photos`, `v-photos`, `video`, `schemes`, `src`, `models`, and `other`. Each one has its own README.
- **2025 code kept** in `src/` for reference, so you can see how the logic evolved.

### 18.2. 🤖 Use of Artificial Intelligence

Ahead of the **WRO 2026 International Final in Puerto Rico**, we are transparently declaring how we used artificial intelligence in this project: which system, for what, and to what extent.

| | Declaration |
| :--- | :--- |
| **System** | Claude (Anthropic), an artificial intelligence assistant |
| **For what** | Repository documentation: organizing and writing the README from our own texts, data, calculations, and photos; translating the documentation into English; integrating the Ackermann steering document into the README; drawing the Mermaid flowcharts; checking links, numbering, and consistency of figures; and preparing the *pull requests*. |
| **To what extent** | Documentation only. The design, construction, and programming of the vehicle and the app, the measurements, the design calculations, the photos, the videos, and the engineering decisions are the team's own work. We review and approve every proposed change before merging it into `main`. |

```mermaid
flowchart LR
    A["👥 Team<br/>design, code, testing,<br/>and measurements"] --> B["📂 Team material<br/>texts, calculations,<br/>photos, and data"]
    B --> C["🤖 Claude<br/>organizes, writes,<br/>translates, and diagrams"]
    C --> D["🔀 Pull request"]
    D --> E{"👥 Team<br/>review"}
    E -- "Changes requested" --> C
    E -- "Approved" --> F["✅ main"]
```

**Basis in the Future Engineers 2026 rules** ([General Rules, January 15, 2026 version](https://wro-association.org/wp-content/uploads/WRO-2026-Future-Engineers-Self-Driving-Cars-General-Rules.pdf)):

| Rule | What it says | How we comply |
| :--- | :--- | :--- |
| **Future Engineers 2026 General Rules** | They contain no rule that prohibits or limits the use of artificial intelligence tools. | Using AI for documentation is allowed; we declare it anyway. |
| **3.3** | *"Coding of the vehicle and its construction (if applicable) may be done only by the team."* | The firmware, the app, the CAD design, and the assembly are the team's work. AI did not program or build the vehicle. |
| **3.7** | A control program that is identical or too similar to solutions sold online is not allowed. | The code in [`src/`](src/) is our own, and its evolution can be followed in the commit history. |
| **Annex C.1** | Judges evaluate *"the clarity and depth of engineering reasoning"*, not appearance or length. | The reasoning (calculations, decisions, and tests) is ours; AI helped us present it clearly. |
| **Future Innovators 2026, rule 6.5** (reference) | In that category, if a team uses AI, it must state which system, for what, and to what extent. | Future Engineers does not require it, but we apply the same standard in this declaration. |
| **4.2 and 4.4** | The official Q&A can clarify or modify the rules, and the judges have the final say on competition day. | We will review the Q&A before the final and answer any questions from the judges about this use. |

---

## 19. 📁 Archive: 2025 Season

This section summarizes the vehicle that earned us **2nd place at the 2025 Venezuelan National Final**. Most of its hardware was replaced in 2026, but we keep this record because it explains the design decisions behind this year's vehicle. Detailed material is collapsed under each subsection.

### 19.1. 🚗 2025 Vehicle Overview

A two-level chassis of our own design, with a Lego rear drivetrain, Lego front steering driven by a servo, and a smartphone camera for obstacle detection.

<table>
<tr>
<td align="center"><img src="v-photos/arriba.JPG" alt="2025 top view" width="230"></td>
<td align="center"><img src="v-photos/frente.JPG" alt="2025 front view" width="230"></td>
<td align="center"><img src="v-photos/lateral-derecho.JPG" alt="2025 right side" width="230"></td>
</tr>
<tr>
<td align="center"><img src="v-photos/lateral-izquierdo.JPG" alt="2025 left side" width="230"></td>
<td align="center"><img src="v-photos/detras.JPG" alt="2025 rear view" width="230"></td>
<td align="center"><img src="v-photos/abajo.JPG" alt="2025 bottom view" width="230"></td>
</tr>
</table>

| Subsystem | 2025 | 2026 |
| :--- | :--- | :--- |
| **Controller** | Arduino at first, then 2 × ESP32 (main + motor slave) | Single ESP32 |
| **Motor driver** | L298N | TB6612FNG |
| **Distance sensing** | 4 × HC-SR04 ultrasonic + 2 × Sharp IR | 4 × VL53L0X ToF + 2 × Sharp IR |
| **Orientation** | MPU6050 + magnetometer | BNO055 |
| **Color sensor** | TCS34725 on the shared bus | TCS34725 on a dedicated I2C bus |
| **Steering** | Lego, parallel geometry | Ackermann, printed in PETG-CF |
| **Chassis** | Two levels, printed by a third party | Monochassis printed in-house |
| **Phone mount** | Modified tripod glued to the chassis | Shark Fin towers with adjustable clamp |
| **Race weight (with phone)** | ≈1300 g | 1092 g |

### 19.2. 🔌 Electronics Evolution

The electrical system went through three versions in 2025 before reaching the current one (version 4, [section 6](#6--power-and-wiring-schematics)).

| Version | Power | Sensors and control |
| :---: | :--- | :--- |
| **V1** | LiPo 2S → XL6009 → L298N → motor. A Mini 360 powered the Arduino, which also powered the servo. | Front and side ultrasonic sensors, diagonal Sharp sensors, gyroscope and color sensor. |
| **V2** | Both Arduinos (main and slave) on one 7 V regulator, plus a separate regulator for the servo and sensors. | A slave Arduino dedicated to the H-bridge, and a start button. |
| **V3** | Two ESP32 boards at 6 V; XL6009 at 7.5 V for the L298N (about 6 V reached the motor); Mini 360 at 5 V for the servo and sensors. | Magnetometer, rear ultrasonic sensor, LED, buzzer, and voltage dividers for the HC-SR04 sensors. |

<details>
<summary><b>2025 schematics (power and sensors, V1–V3)</b></summary>
<br>

| Version | Power | Sensors |
| :---: | :---: | :---: |
| **V1** | <img src="schemes/power_supply_diagram.jpeg" alt="Power V1" width="330"> | <img src="schemes/diagrama-sensores.jpg" alt="Sensors V1" width="330"> |
| **V2** | <img src="schemes/diagrma-alimentacion2.jpg" alt="Power V2" width="330"> | <img src="schemes/diagrama-sensores2.jpg" alt="Sensors V2" width="330"> |
| **V3** | <img src="schemes/Esquema-alimentacio-3.png" alt="Power V3" width="330"> | <img src="schemes/Esquema-alimentacion-4.png" alt="Sensors V3" width="330"> |

**Voltage divider for the HC-SR04.** The `Echo` pin outputs 5 V, while the ESP32 accepts 3.3 V. With R1 = 1 kΩ and R2 = 2 kΩ:

$$V_{out} = 5\text{ V} \cdot \frac{2\text{ kΩ}}{1\text{ kΩ} + 2\text{ kΩ}} \approx 3.33\text{ V}$$

<p align="center"><img src="other/divisor-de-voltaje-abajo.jpg" alt="Voltage dividers soldered on perfboard" width="330"></p>

</details>

### 19.3. 🔧 Chassis Construction

We first modeled a two-level vehicle in Tinkercad, tested the dimensions with a cardboard prototype, refined the tolerances in Fusion 360, and had the parts printed by a third party. The failures of that chassis (bending, vertical motor, manual fixes) are analyzed in the [engineering log](#12--engineering-log-chassis-evolution-v0--v2).

<details>
<summary><b>2025 build process</b></summary>
<br>

| | |
| :---: | :--- |
| <img src="other/base1.jpg" alt="Cardboard base" width="220"> | **Cardboard prototype** used to validate dimensions before printing. |
| <img src="other/modelo3drojo5.jpeg" alt="Tinkercad model" width="220"> | **3D model** with two levels, hexagonal standoffs and space for the battery and switch. |
| <img src="other/impresion3dterminada.jpeg" alt="Printed parts" width="220"> | **Printed parts**, made by an external supplier. |
| <img src="other/modificacion_base_v3.jpeg" alt="Third prototype" width="220"> | **Third prototype**: sensors relocated and new wiring for two ESP32 boards and the magnetometer. |
| <img src="other/fijacion-del-soporte.jfif" alt="Tripod mount" width="220"> | **Phone mount**: a cut-down tripod raised the camera so it could see two pillars at once. Its lever effect later caused the chassis to bend. |

</details>

### 19.4. 📱 Vision App Evolution (V1–V3)

| Version | What it added | Limitation |
| :---: | :--- | :--- |
| **V1** | Red and green detection; sent `R` or `G` to an Arduino over USB. | The main challenge was getting USB communication and OpenCV working on Android. |
| **V2** | Rectangle filtering, distance estimation from object area, and position using a three-zone split of the screen. | Camera perspective made objects appear farther to one side than they were. |
| **V3** | Detection of two objects at once, a case-based `CommandManager`, a left/right screen split and a built-in serial monitor. | Required raising the phone so the camera could see two pillars in a row. |

<details>
<summary><b>App screenshots (V1–V3)</b></summary>
<br>

| V1 | V2 | V3 |
| :---: | :---: | :---: |
| <img src="other/app_prueba.jpg" alt="App V1" width="230"> | <img src="other/Deteccion_con_orientacion (2).jpeg" alt="App V2" width="230"> | <img src="other/Deteccio_doble_app.jpeg" alt="App V3" width="230"> |

</details>

### 19.5. 🧩 Key Problems and Solutions

| Problem | 2025 fix | 2026 solution |
| :--- | :--- | :--- |
| **H-bridge interference:** the car failed to finish turns or missed lines; shielding the wires did not help. | A second microcontroller dedicated to the motor, controlled by the main board. | TB6612FNG and a single ESP32. |
| **Gyroscope drift.** | MPU6050 fused with a magnetometer, which was disturbed by nearby metal and motors. | BNO055 with on-board sensor fusion. |
| **Overshooting 90° turns.** | Turn angle manually tuned down to 70°. | PID turns based on the BNO055 heading. |
| **Missed or double-counted lines.** | 2.4 ms integration time on the TCS34725 and an average of the last 3 readings. | Kept, plus HSV confirmation and reading in a dedicated task. |
| **Imprecise avoidance:** blind reversing after each turn. | Lane system with angle-guided reversing. | Same lane system, using the BNO055 and the rear ToF sensor. |

### 19.6. 🎬 Videos and Source Code

<table>
<tr>
<td align="center" width="33%"><b>Test summary</b><br><br><a href="https://youtu.be/E-xsGcDSaJY?si=9i87w-iJ8TijE_ca"><img src="other/video_promocional.png" alt="2025 test summary" height="150"></a></td>
<td align="center" width="33%"><b>Open challenge laps</b><br><br><a href="https://youtube.com/shorts/wI_OLS1nWhc?si=868zlHss_CnnXe10"><img src="other/prueba-vuelta.png" alt="2025 open challenge" height="150"></a></td>
<td align="center" width="33%"><b>Obstacle challenge</b><br><br><a href="https://www.youtube.com/watch?v=WQM6CjVKSwE"><img src="other/prueba-evacion-obstaculos.png" alt="2025 obstacle challenge" height="150"></a></td>
</tr>
</table>

**2025 source code** (in [`src/`](src/)):

| File | Description |
| :--- | :--- |
| `Wro_Prometheus_Arduino_Maestro_v28.ino` / `Wro_Prometheus_Arduino_Esclavo.ino` | Arduino version (main and motor slave) |
| `Wro_Prometeus_V1.ino` / `Wro_Prometeus_V2_.ino` | ESP32 version with MPU6050 (open and obstacle challenges) |
| `Wro_Prometeus_ESCLAVO_PULLUP_esp32.ino` | ESP32 motor slave |
