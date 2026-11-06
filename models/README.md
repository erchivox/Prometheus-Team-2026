# ⚙️ Modelos 3D

[⬅️ Volver al README principal](../README.md)

Archivos STL de las piezas impresas del vehículo. Todas se diseñaron en **Onshape** y se imprimieron en **PETG**; las piezas críticas sometidas a mayor esfuerzo mecánico (como la dirección Ackermann) van en **PETG-CF**. Los parámetros de impresión están en la sección *9.7 Manufactura aditiva* del README principal.

---

## 🚗 Piezas del vehículo 2026

### 1. Chasis y Estructura

| Archivo | Pieza | Dónde va |
| :--- | :--- | :--- |
| [`Chassis_Lower_Deck.stl`](Chassis_Lower_Deck.stl) | Monochasis (primer piso) | Base principal del vehículo: cama del motor, alojamientos de rodamientos, pivotes de dirección y anclajes de sensores |
| [`Chassis_Upper_Deck.stl`](Chassis_Upper_Deck.stl) | Segundo piso | Nivel superior para montaje de electrónica y base de las torres Shark Fin |
| [`Battery_Holder.stl`](Battery_Holder.stl) | Soporte de batería | Compartimento de sujeción para la batería LiPo 2S en el chasis |
| [`Chassis_Standoff.stl`](Chassis_Standoff.stl) | Separador de chasis | Columnas estructurales entre el primer y segundo piso (con insertos M3) |
| [`Support_Tower_Left.stl`](Support_Tower_Left.stl) | Torre Shark Fin izquierda | Montada sobre el segundo piso (3 insertos M3 en la base) |
| [`Support_Tower_Right.stl`](Support_Tower_Right.stl) | Torre Shark Fin derecha | Montada sobre el segundo piso (3 insertos M3 en la base) |

### 2. Sistema de Dirección Ackermann

| Archivo | Pieza | Dónde va |
| :--- | :--- | :--- |
| [`Steering_Knuckle_Left.stl`](Steering_Knuckle_Left.stl) | Mangueta izquierda | Pivote delantero izquierdo del chasis (con brazo Ackermann integrado) |
| [`Steering_Knuckle_Right.stl`](Steering_Knuckle_Right.stl) | Mangueta derecha | Pivote delantero derecho del chasis (con brazo Ackermann integrado) |
| [`Steering_Tie_Rod.stl`](Steering_Tie_Rod.stl) | Barra de acoplamiento | Une los brazos de ambas manguetas para sincronizar el giro |
| [`Steering_Linkage_Rod.stl`](Steering_Linkage_Rod.stl) | Biela de dirección | Transmite el movimiento desde el brazo del servomotor hacia la barra de acoplamiento |
| [`Tie_Rod_Linkage_Bushing.stl`](Tie_Rod_Linkage_Bushing.stl) | Casquillo de biela / barra | En las articulaciones de la barra de acoplamiento y la biela para eliminar holguras |
| [`Servo_Horn_Bushing.stl`](Servo_Horn_Bushing.stl) | Casquillo del servo | En el agujero a 14 mm del brazo del servomotor (*servo horn*) |
| [`Steering_Servo_Mount.stl`](Steering_Servo_Mount.stl) | Soporte del servomotor | Fija el servomotor MG90 al primer piso del chasis |
| [`Ackermann_Lego_Wheel_Adapter.stl`](Ackermann_Lego_Wheel_Adapter.stl) | Adaptador de rueda delantera | Conecta la rueda LEGO delantera con la varilla lisa de 2.4 mm que atraviesa la mangueta |
| [`Wheel_Shaft_Retaining_Collar.stl`](Wheel_Shaft_Retaining_Collar.stl) | Retenedor de varilla | Extremo libre de la varilla lisa (cara interna de la mangueta) para fijar la rueda axialmente |

### 3. Transmisión y Eje Trasero

| Archivo | Pieza | Dónde va |
| :--- | :--- | :--- |
| [`Motor_Pinion_Gear_15T.stl`](Motor_Pinion_Gear_15T.stl) | Piñón de 15 dientes | Eje del motor 25GA-370 (agujero en "D" a presión, configuración de torque/control) |
| [`Motor_Pinion_Gear_20T.stl`](Motor_Pinion_Gear_20T.stl) | Piñón de 20 dientes | Eje del motor 25GA-370 (agujero en "D" a presión, configuración de mayor velocidad) |
| [`Driven_Spur_Gear_20T.stl`](Driven_Spur_Gear_20T.stl) | Corona recta de 20 dientes | Eje trasero de tracción (configuración de alta velocidad) |
| [`Driven_Spur_Gear_25T.stl`](Driven_Spur_Gear_25T.stl) | Corona recta de 25 dientes | Eje trasero de tracción (configuración de mayor torque) |
| [`Rear_Axle_Lego_Wheel_Adapter.stl`](Rear_Axle_Lego_Wheel_Adapter.stl) | Adaptador de rueda trasera | Extremos del eje trasero para acoplar las ruedas motrices LEGO |
| [`Bearing_Shaft_Adapter_Sleeve.stl`](Bearing_Shaft_Adapter_Sleeve.stl) | Camisa adaptadora de rodamiento | Recubre la varilla lisa del eje trasero para aumentar su diámetro y encajar sin holgura en las rolineras |

### 4. Soportes de Sensores, Cámara y Electrónica

| Archivo | Pieza | Dónde va |
| :--- | :--- | :--- |
| [`Phone_Clamp_Fixed_Base.stl`](Phone_Clamp_Fixed_Base.stl) | Mordaza fija | Entre las torres Shark Fin; pivota para ajustar el ángulo de inclinación del teléfono |
| [`Phone_Clamp_Sliding_Jaw.stl`](Phone_Clamp_Sliding_Jaw.stl) | Mordaza móvil | Se desliza dentro de la mordaza fija para sujetar el teléfono |
| [`Phone_Clamp_Lead_Screw.stl`](Phone_Clamp_Lead_Screw.stl) | Tornillo de ajuste M10 | Rosca impresa que atraviesa la base fija y empuja la mordaza móvil |
| [`Sharp_Sensor_Mount.stl`](Sharp_Sensor_Mount.stl) | Soporte de sensor Sharp | Anclaje en el chasis para los sensores infrarrojos GP2Y0A21 |
| [`ToF_Sensor_Mount_High.stl`](ToF_Sensor_Mount_High.stl) | Soporte alto para ToF | Elevador/soporte superior para sensores láser VL53L0X |
| [`ToF_Sensor_Mount_Low.stl`](ToF_Sensor_Mount_Low.stl) | Soporte bajo para ToF | Elevador/soporte inferior para sensores láser VL53L0X |
| [`PCB_Pillar_Standoff.stl`](PCB_Pillar_Standoff.stl) | Pilar separador de placa | Columna de elevación para aislar y fijar las placas electrónicas (baquelita/PCB) |
| [`PCB_Spacer_Mount_Base.stl`](PCB_Spacer_Mount_Base.stl) | Base de soporte para placa | Base inferior de fijación para los separadores de la electrónica |
| [`PCB_Spacer_Mount_Top_Cap.stl`](PCB_Spacer_Mount_Top_Cap.stl) | Tapa de soporte para placa | Pieza superior de cierre para asegurar la placa electrónica al soporte |

---

## 📁 Piezas de 2025

| Archivo | Pieza |
| :--- | :--- |
| [`primer_piso_vehiculo_ajustado.stl`](primer_piso_vehiculo_ajustado.stl) | Primer piso del chasis 2025 |
| [`segundo_piso_vehiculo_ajustado.stl`](segundo_piso_vehiculo_ajustado.stl) | Segundo piso del chasis 2025 |
| [`soporte_servo.stl`](soporte_servo.stl) | Soporte del servo de la dirección Lego |
| [`soporte_HC-SR04.stl`](soporte_HC-SR04.stl) | Soporte del sensor ultrasónico HC-SR04 |
| [`aleron_vehiculo.stl`](aleron_vehiculo.stl) | Alerón decorativo |
