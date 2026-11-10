# ⚙️ 3D Models

[⬅️ Back to the main README](../README.md)

STL files for the vehicle's printed parts. All of them were designed in **Onshape** and printed in **PETG**; the critical parts under the most mechanical stress (such as the Ackermann steering) are printed in **PETG-CF**. The print settings are in section *9.7 Additive Manufacturing* of the main README.

---

## 🚗 2026 Vehicle Parts

### 1. Chassis and Structure

| File | Part | Where it goes |
| :--- | :--- | :--- |
| [`Chassis_Lower_Deck.stl`](Chassis_Lower_Deck.stl) | Monochassis (first level) | Main base of the vehicle: motor cradle, bearing housings, steering pivots, and sensor mounts |
| [`Chassis_Upper_Deck.stl`](Chassis_Upper_Deck.stl) | Second level | Upper level for mounting the electronics and base for the Shark Fin towers |
| [`Battery_Holder.stl`](Battery_Holder.stl) | Battery holder | Compartment that holds the 2S LiPo battery on the chassis |
| [`Chassis_Standoff.stl`](Chassis_Standoff.stl) | Chassis standoff | Structural columns between the first and second levels (with M3 inserts) |
| [`Support_Tower_Left.stl`](Support_Tower_Left.stl) | Left Shark Fin tower | Mounted on the second level (3 M3 inserts in the base) |
| [`Support_Tower_Right.stl`](Support_Tower_Right.stl) | Right Shark Fin tower | Mounted on the second level (3 M3 inserts in the base) |

### 2. Ackermann Steering System

| File | Part | Where it goes |
| :--- | :--- | :--- |
| [`Steering_Knuckle_Left.stl`](Steering_Knuckle_Left.stl) | Left steering knuckle | Front left pivot of the chassis (with a built-in Ackermann arm) |
| [`Steering_Knuckle_Right.stl`](Steering_Knuckle_Right.stl) | Right steering knuckle | Front right pivot of the chassis (with a built-in Ackermann arm) |
| [`Steering_Tie_Rod.stl`](Steering_Tie_Rod.stl) | Tie rod | Connects the arms of both knuckles so they turn in sync |
| [`Steering_Linkage_Rod.stl`](Steering_Linkage_Rod.stl) | Steering link | Transmits the motion from the servo horn to the tie rod |
| [`Tie_Rod_Linkage_Bushing.stl`](Tie_Rod_Linkage_Bushing.stl) | Link / tie rod bushing | In the tie rod and link joints, to eliminate play |
| [`Servo_Horn_Bushing.stl`](Servo_Horn_Bushing.stl) | Servo bushing | In the hole 14 mm from the center of the *servo horn* |
| [`Steering_Servo_Mount.stl`](Steering_Servo_Mount.stl) | Servo mount | Attaches the MG90 servo to the first level of the chassis |
| [`Ackermann_Lego_Wheel_Adapter.stl`](Ackermann_Lego_Wheel_Adapter.stl) | Front wheel adapter | Connects the front LEGO wheel to the 2.4 mm smooth rod that passes through the knuckle |
| [`Wheel_Shaft_Retaining_Collar.stl`](Wheel_Shaft_Retaining_Collar.stl) | Rod retainer | Free end of the smooth rod (inner side of the knuckle) to hold the wheel axially |

### 3. Drivetrain and Rear Axle

| File | Part | Where it goes |
| :--- | :--- | :--- |
| [`Motor_Pinion_Gear_15T.stl`](Motor_Pinion_Gear_15T.stl) | 15-tooth pinion | 25GA-370 motor shaft (press-fit "D" bore, torque/control configuration) |
| [`Motor_Pinion_Gear_20T.stl`](Motor_Pinion_Gear_20T.stl) | 20-tooth pinion | 25GA-370 motor shaft (press-fit "D" bore, higher-speed configuration) |
| [`Driven_Spur_Gear_20T.stl`](Driven_Spur_Gear_20T.stl) | 20-tooth spur gear | Rear drive axle (high-speed configuration) |
| [`Driven_Spur_Gear_25T.stl`](Driven_Spur_Gear_25T.stl) | 25-tooth spur gear | Rear drive axle (higher-torque configuration) |
| [`Rear_Axle_Lego_Wheel_Adapter.stl`](Rear_Axle_Lego_Wheel_Adapter.stl) | Rear wheel adapter | Ends of the rear axle, to attach the LEGO drive wheels |
| [`Bearing_Shaft_Adapter_Sleeve.stl`](Bearing_Shaft_Adapter_Sleeve.stl) | Bearing adapter sleeve | Wraps the smooth rod of the rear axle to increase its diameter so it fits the bearings with no play |

### 4. Sensor, Camera, and Electronics Mounts

| File | Part | Where it goes |
| :--- | :--- | :--- |
| [`Phone_Clamp_Fixed_Base.stl`](Phone_Clamp_Fixed_Base.stl) | Fixed jaw | Between the Shark Fin towers; pivots to adjust the phone's tilt angle |
| [`Phone_Clamp_Sliding_Jaw.stl`](Phone_Clamp_Sliding_Jaw.stl) | Sliding jaw | Slides inside the fixed jaw to hold the phone |
| [`Phone_Clamp_Lead_Screw.stl`](Phone_Clamp_Lead_Screw.stl) | M10 adjustment screw | Printed thread that goes through the fixed base and pushes the sliding jaw |
| [`Sharp_Sensor_Mount.stl`](Sharp_Sensor_Mount.stl) | Sharp sensor mount | Chassis mount for the GP2Y0A21 infrared sensors |
| [`ToF_Sensor_Mount_High.stl`](ToF_Sensor_Mount_High.stl) | High ToF mount | Riser/upper mount for the VL53L0X laser sensors |
| [`ToF_Sensor_Mount_Low.stl`](ToF_Sensor_Mount_Low.stl) | Low ToF mount | Riser/lower mount for the VL53L0X laser sensors |
| [`PCB_Pillar_Standoff.stl`](PCB_Pillar_Standoff.stl) | Board pillar standoff | Riser column to isolate and secure the electronic boards (perfboard/PCB) |
| [`PCB_Spacer_Mount_Base.stl`](PCB_Spacer_Mount_Base.stl) | Board mount base | Lower base that holds the electronics standoffs |
| [`PCB_Spacer_Mount_Top_Cap.stl`](PCB_Spacer_Mount_Top_Cap.stl) | Board mount cap | Upper closing piece that secures the electronic board to the mount |

---

## 📁 2025 Parts

| File | Part |
| :--- | :--- |
| [`primer_piso_vehiculo_ajustado.stl`](primer_piso_vehiculo_ajustado.stl) | 2025 chassis, first level |
| [`segundo_piso_vehiculo_ajustado.stl`](segundo_piso_vehiculo_ajustado.stl) | 2025 chassis, second level |
| [`soporte_servo.stl`](soporte_servo.stl) | Servo mount for the LEGO steering |
| [`soporte_HC-SR04.stl`](soporte_HC-SR04.stl) | HC-SR04 ultrasonic sensor mount |
| [`aleron_vehiculo.stl`](aleron_vehiculo.stl) | Decorative spoiler |
