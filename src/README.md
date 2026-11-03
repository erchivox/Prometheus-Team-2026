# 💾 Código del vehículo

[⬅️ Volver al README principal](../README.md)

Firmware del ESP32 (IDE de Arduino) y enlaces al código de la app de visión. La lógica de navegación está explicada en la sección *15. Software y control* del README principal.

---

## ✅ Firmware 2026

| Archivo | Desafío | Qué hace |
| :--- | :--- | :--- |
| [`v1PrometeusTeam2026.ino`](v1PrometeusTeam2026.ino) | 🏁 Vuelta libre | Seguimiento de pared con ToF y BNO055, conteo de líneas y giros con PID |
| [`v2PrometrusTeam2026.ino`](v2PrometrusTeam2026.ino) | 🚧 Obstáculos | Todo lo anterior más la salida del estacionamiento, el sistema de carriles y los comandos de la app por USB serial |

### 📚 Librerías

| Librería | Uso | Instalación |
| :--- | :--- | :--- |
| `Wire.h` | Buses I2C (#1 en 21/22 y #2 en 32/33) | Incluida con el core del ESP32 |
| [`Adafruit_BNO055`](https://github.com/adafruit/Adafruit_BNO055) + [`Adafruit_Sensor`](https://github.com/adafruit/Adafruit_Sensor) | IMU BNO055 (orientación) | Gestor de librerías: "Adafruit BNO055" |
| [`Adafruit_VL53L0X`](https://github.com/adafruit/Adafruit_VL53L0X) | 4 sensores ToF | Gestor de librerías: "Adafruit VL53L0X" |
| [`Adafruit_TCS34725`](https://github.com/adafruit/Adafruit_TCS34725) | Sensor de color | Gestor de librerías: "Adafruit TCS34725" |
| [`Adafruit_NeoPixel`](https://github.com/adafruit/Adafruit_NeoPixel) | Tira de LEDs de estado | Gestor de librerías: "Adafruit NeoPixel" |
| [`ESP32Servo`](https://github.com/madhephaestus/ESP32Servo) | Servo de la dirección | Gestor de librerías: "ESP32Servo" |
| [`ColorConverterLib`](https://github.com/luisllamasbinaburo/Arduino-ColorConverter) | Conversión RGB → HSV | Descargar el .zip e instalar con *Sketch → Incluir librería → Añadir .ZIP* |

### ⬆️ Cargar el código

1. Instale el **IDE de Arduino** y agregue el soporte para **ESP32** (*Gestor de tarjetas → esp32 by Espressif Systems*).
2. Instale las librerías de la tabla anterior.
3. Abra `v1PrometeusTeam2026.ino` o `v2PrometrusTeam2026.ino`.
4. Seleccione la placa **ESP32 Dev Module** y el puerto correspondiente.
5. Presione **Verificar** y luego **Subir**.
6. Con el vehículo en la pista, pase el interruptor de inicio (D23) a LOW para arrancar.

---

## 📁 Firmware 2025

| Archivo | Descripción |
| :--- | :--- |
| [`Wro_Prometheus_Arduino_Maestro_v28.ino`](Wro_Prometheus_Arduino_Maestro_v28.ino) | Arduino principal (sensores y lógica) |
| [`Wro_Prometheus_Arduino_Esclavo.ino`](Wro_Prometheus_Arduino_Esclavo.ino) | Arduino esclavo para el puente H |
| [`Wro_Prometeus_V1.ino`](Wro_Prometeus_V1.ino) | ESP32 con MPU6050, vuelta libre |
| [`Wro_Prometeus_V2_.ino`](Wro_Prometeus_V2_.ino) | ESP32 con MPU6050, obstáculos (primer sistema de carriles) |
| [`Wro_Prometeus_ESCLAVO_PULLUP_esp32.ino`](Wro_Prometeus_ESCLAVO_PULLUP_esp32.ino) | ESP32 esclavo del motor |

Estas versiones usan además la librería [`MPU6050`](https://github.com/ElectronicCats/mpu6050) (Electronic Cats).

---

## 📱 App WRO Prometheus (Android Studio)

El código de la aplicación **WRO Prometheus** está en su propio repositorio: [APP-WRO-PROMETHEUS-TEAM](https://github.com/SamFlips/APP-WRO-PROMETHEUS-TEAM/tree/master) ([descarga en .zip](https://drive.google.com/file/d/1VvdzdWbxGnchFslUwI8wQfkkwYPTyGRD/view?usp=drive_link)). La app, que sirve para procesar datos de imágenes para nuestro Vehículo Autónomo en la competencia. Aquí encontrarás toda la información relevante sobre las tecnologías utilizadas y cómo preparar el proyecto.

---

### 📦 Tecnologías y dependencias principales

Nuestra aplicación Android ha sido desarrollada utilizando **Android Studio** y se basa en las siguientes librerías y marcos de trabajo:

#### Librerías de AndroidX y Material Design

Componentes fundamentales de la plataforma Android para la interfaz de usuario y funcionalidades básicas, asegurando compatibilidad y una experiencia de usuario moderna.

* `androidx.core:core-ktx`: [Documentación oficial de AndroidX Core KTX](https://developer.android.com/kotlin/ktx)
* `androidx.appcompat:appcompat`: [Notas de lanzamiento de AndroidX Appcompat](https://developer.android.com/jetpack/androidx/releases/appcompat)
* `com.google.android.material:material`: [Guía de inicio de Material Design Components para Android](https://material.io/develop/android/docs/getting-started)
* `androidx.constraintlayout:constraintlayout`: [Notas de lanzamiento de AndroidX ConstraintLayout](https://developer.android.com/jetpack/androidx/releases/constraintlayout)

#### CameraX

Un *toolkit* de Android Jetpack que simplifica el uso de la cámara del dispositivo, proporcionando una API consistente y fácil de usar para diversas funcionalidades de cámara.

* `androidx.camera:camera-core`
* `androidx.camera:camera-camera2`
* `androidx.camera:camera-lifecycle`
* `androidx.camera:camera-view`
* Puedes encontrar la documentación y ejemplos en la **[Guía general de CameraX](https://developer.android.com/media/camera/camerax)**.

#### USB Serial for Android

Una librería esencial para la comunicación de nuestra aplicación con el microcontrolador del vehículo a través de una conexión serial USB.

* `com.github.mik3y:usb-serial-for-android`: Puedes consultar el repositorio oficial en **[GitHub - usb-serial-for-android](https://github.com/mik3y/usb-serial-for-android)**.

#### OpenCV (Open Source Computer Vision Library)

Una potente librería para el procesamiento de imágenes y visión por computador, utilizada para tareas de análisis visual y toma de decisiones dentro de la aplicación.

* **Integración Local**: Es importante destacar que **OpenCV está integrado como un módulo local dentro de este proyecto de Android Studio**. Esto significa que los archivos binarios y la configuración necesaria de OpenCV ya están incluidos en este repositorio.
* `implementation(project(":OpenCV"))`: Esta línea en el `build.gradle` indica que el módulo `:OpenCV` es una parte intrínseca de este proyecto.
* Para documentación general sobre la librería, puedes visitar el sitio oficial de **[OpenCV](https://opencv.org/)**.

#### Librerías de Testing

Componentes estándar para realizar pruebas unitarias y de instrumentación en la aplicación.

* `junit:junit`: [Sitio oficial de JUnit 4](https://junit.org/junit4/faq.html)
* `androidx.test.ext:junit`: [Notas de lanzamiento de AndroidX Test Ext JUnit](https://developer.android.com/jetpack/androidx/releases/test)
* `androidx.test.espresso:espresso-core`: [Notas de lanzamiento de AndroidX Test Espresso](https://developer.android.com/jetpack/androidx/releases/espresso)

---

### 🛠️ Preparación del proyecto Android

Para compilar y ejecutar esta aplicación en su entorno, siga estos pasos:

1.  **Requisitos Previos**:
    * Asegúrese de tener **Android Studio** instalado y configurado, incluyendo el SDK de Android necesario y las herramientas de Gradle.

2.  **Abrir el Proyecto**:
    * Clone el repositorio de la app y ábralo en Android Studio.

3.  **Sincronización de Gradle**:
    * Android Studio detectará automáticamente el proyecto y sus archivos `build.gradle`. Se le pedirá que sincronice el proyecto. Este paso es crucial, ya que Gradle descargará automáticamente todas las dependencias externas (AndroidX, CameraX, USB Serial, etc.) desde los repositorios de Maven configurados.
    * **No se requiere instalación de OpenCV**: Dado que OpenCV se incluye como un módulo local, no necesita realizar ninguna instalación o configuración adicional de OpenCV en su sistema; Gradle lo gestionará directamente desde el código fuente del proyecto. Simplemente asegúrese de que la estructura de carpetas incluya el módulo `:OpenCV` y que esté referenciado en el `settings.gradle`.

4.  **Compilar y Ejecutar**:
    * Una vez que la sincronización de Gradle finalice sin errores, podrá compilar el proyecto y ejecutar la aplicación en su dispositivo Android.
