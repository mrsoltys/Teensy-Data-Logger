# 🌡️ Teensy 4.1 Environmental Data Logger

This project logs temperature, humidity, and pressure data from a **SparkFun BME280** environmental sensor and timestamps each reading using a **SparkFun RV-1805 Real-Time Clock (RTC)**.  
Data are stored on a microSD card in CSV format, suitable for long-term environmental monitoring.

This readme.m file was created with AI. 

---

## 🧰 Hardware Overview

| Component | Description | Link |
|------------|--------------|------|
| **Teensy 4.1** | Main microcontroller with built-in SD slot | [PJRC Teensy 4.1](https://www.pjrc.com/store/teensy41.html) |
| **SparkFun BME280** | Measures temperature (°C), humidity (%), and pressure (Pa) via I²C | [SparkFun BME280 (SEN-13676)](https://www.sparkfun.com/products/13676) |
| **SparkFun RV-1805** | Low-power real-time clock for accurate timestamps | [SparkFun RV-1805 (BOB-25268)](https://www.sparkfun.com/products/25268) |
| **Button (Momentary)** | Starts/stops data recording | — |
| **LED (built-in)** | Blinks while recording, steady ON when idle | — |
| **microSD Card** | Stores CSV log files (FAT32) | — |

---

## ⚡ Wiring Diagram

| Connection | Teensy 4.1 Pin | RV-1805 Pin | BME280 Pin | Notes |
|-------------|----------------|--------------|-------------|--------|
| **3.3V Power** | 3.3V | VCC | VCC | Both sensors are 3.3 V devices |
| **Ground** | GND | GND | GND | Common ground |
| **I²C SDA** | 18 | SDA | SDA | Shared I²C data line |
| **I²C SCL** | 19 | SCL | SCL | Shared I²C clock line |
| **Button Input** | 28 | — | — | Pulled up internally (to start/stop) |
| **Button Ground** | 31 | — | — | Ground side of button |
| **LED** | 13 | — | — | Built-in LED indicator |

> **Note:** I²C pull-up resistors are typically present on SparkFun breakout boards.  
> If you use bare sensors, add 4.7 kΩ pull-ups to 3.3 V on SDA/SCL.

---

## 📄 CSV Log Format

Each record contains:
datetime,temp_C,humidity_pct,pressure_Pa,millis
2025-10-27 16:41:23,25.42,44.31,101234.11,12345678


---

## ⏱️ Sample Rate Guidelines

The logger supports a wide range of sampling rates, but physical and timing limits make certain ranges more practical.

| Range | Typical Sample Rate | Trigger Method | Description | Max Continuous Runtime* |
|--------|---------------------|----------------|--------------|---------------------------|
| **Fast** | 10–20 Hz | `IntervalTimer` | Near upper limit for reliable BME280 + RTC reads | ~49 days (millis() rollover) |
| **Moderate** | 0.1–5 Hz | `IntervalTimer` | Ideal for environmental logging | Months (restart before 4 GB file) |
| **Slow** | 0.001–0.1 Hz (1 sample/10–1000 s) | `millis()` loop | Uses loop timing instead of interrupts | Months |
| **Very Slow** | <0.001 Hz (hours–days) | `millis()` + RTC | For long-term logging; drift negligible | Years (with periodic file rotation) |

\* Max runtime refers to continuous operation before either `millis()` wraps (≈49.7 days) or file rotation is recommended.  
Files are limited to **4 GB** due to FAT32 constraints.

---

## 🧮 Data Volume Estimate

At roughly **60 bytes/sample**:

| Sample Rate | Data per Day | Time to Fill 4 GB |
|--------------|---------------|--------------------|
| 0.1 Hz (1/10 s) | ~0.5 MB/day | ~22 years |
| 1 Hz | ~5 MB/day | ~2.2 years |
| 10 Hz | ~50 MB/day | ~80 days |

---

## 💾 File Naming and Storage

- Each new recording session creates a sequential file:  
  `LOG000.CSV`, `LOG001.CSV`, etc.  
- Files are saved to the SD card’s root directory.
- A 512-byte write buffer minimizes SD wear and latency.

---

## 🧠 Known Limitations

- **`millis()` overflows** every 49.7 days (safe but resets the counter).  
- **FAT32 limit:** Max file size is **4 GB**.  
- **RTC drift:** May gain/lose a few seconds per month; periodically resync to a reference time.  
- **I²C lockups (rare):** Add a watchdog reset for long unattended deployments.

---

## 🚀 Future Improvements

- Add **automatic daily file rotation**.  
- Implement a **watchdog reset** for unattended long-term logging.  
- Support **SPI sensors** for higher sample rates.  
- Add **low-power sleep mode** for remote deployments.

---

## 📚 License and Credits

- Developed for Teensy 4.1 using Arduino framework.  
- Libraries:  
  - [SparkFun RV-1805 Arduino Library](https://github.com/sparkfun/SparkFun_RV-1805_Arduino_Library)  
  - [SparkFun BME280 Arduino Library](https://github.com/sparkfun/SparkFun_BME280_Arduino_Library)  
- Example code and modifications by [PJRC](https://www.pjrc.com/) and SparkFun Electronics.  

---

### 🧩 Version
- Firmware: `v1.0`
- Date: October 2025
- Author: Mike Soltys


