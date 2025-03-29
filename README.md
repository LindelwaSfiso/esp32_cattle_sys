# Project Documentation: ESP32 Cattle Weighting System

## Introduction
This document provides a comprehensive guide to the "ESP32 Cattle Weighting System" project. It details the project's setup, how Django operates within it, the role of APIs, and an explanation of the Django folder structure. This project is aimed at cattle farmers who what to keep track of the cattle weight using an innovative and digital solution.

---

## 2. Project Requirements
- **Python 3.12:** Programming language for the backend.
- **Django:** Web framework for the backend server.
- **DjangoRestFramework:** Framework for creating APIs.
- **Arduino/ESP32/C++:** Used for the micro-controller aspect of the project.

---

## 3. Installation and Setup

### Backend Setup:
1. Clone the repository:
    ```shell
    $ git clone https://github.com/LindelwaSfiso/esp32_cattle_sys.git
    ```

2. Navigate to the server directory:
    ```shell
    $ cd server
    ```

3. Create a virtual environment and activate it:
    ```shell
    $ python -3.12 -m venv venv
    $ venv\scripts\activate
    ```

4. Install the required packages:
    ```shell
    $ (venv) pip install -r requirements.txt
    ```

5. Apply database migrations:
    ```shell
    $ (venv) manage.py migrate
    ```

6. Run the server:
    ```shell
    $ (venv) manage.py runserver 0.0.0.0:8000
    ```

### Backend CSS Files Setup:
1. Navigate to the server directory:
    ```shell
    $ cd server
    ```

2. Build the CSS files:
    ```shell
    $ npm run build # generates build CSS for the website
    ```

### Creating Administration Account:
1. Navigate to the admin directory:
    ```shell
    $ cd server
    ```

2. Activate the virtual environment:
    ```shell
    $ venv\scripts\activate
    ```

3. Create a superuser account:
    ```shell
    $ (venv) manage.py createsuperuser
    ```

---

## 4. ESP32 Setup
Follow the [ESP32 devtools setup guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html) for installing and configuring the ESP32 microcontroller.

---



# References

1. https://randomhnerdtutorials.com/esp32-load-cell-hx711/
2. https://www.electronicwings.com/esp32/rfid-rc522-interfacing-with-esp32
3. https://esp32io.com/tutorials/esp32-servo-motor
3. https://randomnerdtutorials.com/esp32-esp8266-i2c-lcd-arduino-ide/
4. https://github.com/marcoschwartz/LiquidCrystal_I2C/archive/master.zip

# Arduino IDE Libraries
1. https://github.com/OSSLibraries/Arduino_MFRC522v2 MFRC522v2
2. https://github.com/bogde/HX711
3. esp32 board version 3.0.7
4. https://github.com/madhephaestus/ESP32Servo

