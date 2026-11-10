# 💾 Vehicle Code

[⬅️ Back to the main README](../README.md)

ESP32 firmware (Arduino IDE) and links to the vision app's code. The navigation logic is explained in section *14. Software and Control* of the main README.

---

## ✅ 2026 Firmware

| File | Challenge | What it does |
| :--- | :--- | :--- |
| [`v1PrometeusTeam2026.ino`](v1PrometeusTeam2026.ino) | 🏁 Open lap | Wall following with ToF and the BNO055, line counting, and PID turns |
| [`v2PrometrusTeam2026.ino`](v2PrometrusTeam2026.ino) | 🚧 Obstacles | Everything above, plus leaving the parking area, the lane system, and app commands over USB serial |

### 📚 Libraries

| Library | Purpose | Installation |
| :--- | :--- | :--- |
| `Wire.h` | I2C buses (#1 on 21/22 and #2 on 32/33) | Included with the ESP32 core |
| [`Adafruit_BNO055`](https://github.com/adafruit/Adafruit_BNO055) + [`Adafruit_Sensor`](https://github.com/adafruit/Adafruit_Sensor) | BNO055 IMU (orientation) | Library Manager: "Adafruit BNO055" |
| [`Adafruit_VL53L0X`](https://github.com/adafruit/Adafruit_VL53L0X) | 4 ToF sensors | Library Manager: "Adafruit VL53L0X" |
| [`Adafruit_TCS34725`](https://github.com/adafruit/Adafruit_TCS34725) | Color sensor | Library Manager: "Adafruit TCS34725" |
| [`Adafruit_NeoPixel`](https://github.com/adafruit/Adafruit_NeoPixel) | Status LED strip | Library Manager: "Adafruit NeoPixel" |
| [`ESP32Servo`](https://github.com/madhephaestus/ESP32Servo) | Steering servo | Library Manager: "ESP32Servo" |
| [`ColorConverterLib`](https://github.com/luisllamasbinaburo/Arduino-ColorConverter) | RGB → HSV conversion | Download the .zip and install it with *Sketch → Include Library → Add .ZIP Library* |

### ⬆️ Uploading the Code

1. Install the **Arduino IDE** and add **ESP32** support (*Boards Manager → esp32 by Espressif Systems*).
2. Install the libraries in the table above.
3. Open `v1PrometeusTeam2026.ino` or `v2PrometrusTeam2026.ino`.
4. Select the **ESP32 Dev Module** board and the correct port.
5. Click **Verify** and then **Upload**.
6. With the vehicle on the track, switch the start switch (D23) to LOW to start.

---

## 📁 2025 Firmware

| File | Description |
| :--- | :--- |
| [`Wro_Prometheus_Arduino_Maestro_v28.ino`](Wro_Prometheus_Arduino_Maestro_v28.ino) | Main Arduino (sensors and logic) |
| [`Wro_Prometheus_Arduino_Esclavo.ino`](Wro_Prometheus_Arduino_Esclavo.ino) | Secondary Arduino for the H-bridge |
| [`Wro_Prometeus_V1.ino`](Wro_Prometeus_V1.ino) | ESP32 with MPU6050, open lap |
| [`Wro_Prometeus_V2_.ino`](Wro_Prometeus_V2_.ino) | ESP32 with MPU6050, obstacles (first lane system) |
| [`Wro_Prometeus_ESCLAVO_PULLUP_esp32.ino`](Wro_Prometeus_ESCLAVO_PULLUP_esp32.ino) | Secondary ESP32 for the motor |

These versions also use the [`MPU6050`](https://github.com/ElectronicCats/mpu6050) library (Electronic Cats).

---

## 📱 WRO Prometheus App (Android Studio)

The code for the **WRO Prometheus** app lives in its own repository: [APP-WRO-PROMETHEUS-TEAM](https://github.com/SamFlips/APP-WRO-PROMETHEUS-TEAM/tree/master) ([.zip download](https://drive.google.com/file/d/1VvdzdWbxGnchFslUwI8wQfkkwYPTyGRD/view?usp=drive_link)). The app processes image data for our autonomous vehicle during the competition. Here you will find all the relevant information about the technologies we used and how to set up the project.

---

### 📦 Main Technologies and Dependencies

Our Android app was built with **Android Studio** and relies on the following libraries and frameworks:

#### AndroidX and Material Design Libraries

Core Android platform components for the user interface and basic functionality, ensuring compatibility and a modern user experience.

* `androidx.core:core-ktx`: [AndroidX Core KTX official documentation](https://developer.android.com/kotlin/ktx)
* `androidx.appcompat:appcompat`: [AndroidX Appcompat release notes](https://developer.android.com/jetpack/androidx/releases/appcompat)
* `com.google.android.material:material`: [Getting started with Material Design Components for Android](https://material.io/develop/android/docs/getting-started)
* `androidx.constraintlayout:constraintlayout`: [AndroidX ConstraintLayout release notes](https://developer.android.com/jetpack/androidx/releases/constraintlayout)

#### CameraX

An Android Jetpack *toolkit* that simplifies using the device's camera, providing a consistent, easy-to-use API for a range of camera features.

* `androidx.camera:camera-core`
* `androidx.camera:camera-camera2`
* `androidx.camera:camera-lifecycle`
* `androidx.camera:camera-view`
* You can find documentation and examples in the **[CameraX overview](https://developer.android.com/media/camera/camerax)**.

#### USB Serial for Android

An essential library for communication between our app and the vehicle's microcontroller over a USB serial connection.

* `com.github.mik3y:usb-serial-for-android`: see the official repository at **[GitHub - usb-serial-for-android](https://github.com/mik3y/usb-serial-for-android)**.

#### OpenCV (Open Source Computer Vision Library)

A powerful library for image processing and computer vision, used for visual analysis and decision-making inside the app.

* **Local integration:** note that **OpenCV is integrated as a local module inside this Android Studio project**. This means the OpenCV binaries and required configuration are already included in the repository.
* `implementation(project(":OpenCV"))`: this line in `build.gradle` indicates that the `:OpenCV` module is an integral part of the project.
* For general documentation on the library, visit the official **[OpenCV](https://opencv.org/)** website.

#### Testing Libraries

Standard components for running unit and instrumented tests on the app.

* `junit:junit`: [JUnit 4 official website](https://junit.org/junit4/faq.html)
* `androidx.test.ext:junit`: [AndroidX Test Ext JUnit release notes](https://developer.android.com/jetpack/androidx/releases/test)
* `androidx.test.espresso:espresso-core`: [AndroidX Test Espresso release notes](https://developer.android.com/jetpack/androidx/releases/espresso)

---

### 🛠️ Setting Up the Android Project

To build and run the app in your environment, follow these steps:

1.  **Prerequisites**:
    * Make sure **Android Studio** is installed and configured, including the required Android SDK and Gradle tools.

2.  **Open the project**:
    * Clone the app repository and open it in Android Studio.

3.  **Gradle sync**:
    * Android Studio will automatically detect the project and its `build.gradle` files and ask you to sync the project. This step is crucial, because Gradle will automatically download all external dependencies (AndroidX, CameraX, USB Serial, etc.) from the configured Maven repositories.
    * **No OpenCV installation required**: since OpenCV is included as a local module, you do not need to install or configure OpenCV on your system; Gradle handles it directly from the project source. Just make sure the folder structure includes the `:OpenCV` module and that it is referenced in `settings.gradle`.

4.  **Build and run**:
    * Once the Gradle sync finishes without errors, you can build the project and run the app on your Android device.
