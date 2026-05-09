# BIKEYE
BIKEYE — A smart IoT-based bike theft detection system using ESP32-CAM, accelerometer, Telegram alerts, real-time image capture, and servo-based ignition cut-off.
# 🔐 BIKEYE — Bike Theft Monitoring System

> An intelligent IoT-based security solution that protects motorcycles and bicycles from theft using real-time image capture, motion detection, instant Telegram alerts, and automatic ignition immobilization.

---

## 📌 Overview

**BIKEYE** is a smart anti-theft system built around the **ESP32-CAM** microcontroller. It continuously monitors your bike for unauthorized movement. When tampering is detected, it captures photographic evidence, triggers an alarm, sends an instant alert to your phone via **Telegram**, and locks the ignition using a **servo motor** — all automatically.

---

## ⚙️ Features

- 📷 **Real-time Image Capture** — ESP32-CAM captures images of the intruder upon motion detection
- 📲 **Telegram Bot Alerts** — Instantly sends captured images and a "Movement Detected" warning to the owner's phone
- 🔔 **Buzzer Alarm** — Audible alert to warn nearby people
- 🔒 **Ignition Cut-off** — Servo motor disconnects/locks the ignition to immobilize the bike
- ⚡ **Low Power** — Runs on a rechargeable battery, suitable for always-on monitoring
- 🌐 **IoT-based Remote Monitoring** — Monitor your bike from anywhere in real time

---

## 🧰 Hardware Components

| Component        | Purpose                                      |
|------------------|----------------------------------------------|
| ESP32-CAM        | Microcontroller + integrated camera module   |
| Accelerometer    | Detects vibrations and unauthorized movement |
| Buzzer           | Audible alarm on theft detection             |
| Servo Motor      | Ignition cut-off / immobilization mechanism  |
| Switch           | Manual ON/OFF control                        |
| Rechargeable Battery | Powers the entire system                |

---

## 🔄 How It Works

```
Bike is stationary
       │
       ▼
Accelerometer monitors continuously
       │
  Movement detected?
       │
      YES
       │
       ├──► ESP32-CAM captures image of intruder
       │
       ├──► Buzzer activates (audible alarm)
       │
       ├──► Telegram bot sends image + "Movement Detected" alert to owner
       │
       └──► Servo motor locks ignition (bike immobilized)
```

---

## 📡 Tech Stack

- **Microcontroller:** ESP32-CAM
- **Communication:** Wi-Fi (Telegram Bot API)
- **Sensor:** ADXL345 / MPU6050 Accelerometer
- **Actuator:** Servo Motor (SG90 or similar)
- **Notification Platform:** Telegram Bot
- **Programming Language:** C++ (Arduino IDE)

---


## 📁 Project Structure

```
BIKEYE/
├── src/
│   └── bikeye.ino          # Main Arduino sketch
├── circuit/
│   └── schematic.png       # Circuit diagram
├── docs/
│   └── report.pdf          # Project documentation
├── images/
│   └── setup.jpg           # Hardware setup photos
└── README.md
```



---

## 🛡️ Security & Privacy

- Images are only captured and transmitted upon confirmed motion detection
- All alerts are sent exclusively to the registered owner's Telegram chat ID
- No data is stored on external servers

---

## 🤝 Contributing

Pull requests are welcome! For major changes, please open an issue first to discuss what you would like to change.

---


## 👤 Author

**Your Name**  
📧 gjashwanthreddy18@gmail.com.com  
🔗 [GitHub](https://github.com/jashwanth018)

---

> *BIKEYE — Because your bike deserves an eye.* 👁️
