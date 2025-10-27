# Teensy Data Logger Example Codes

The goal of this project is to create an **open-source, low-cost, portable Arduino-based data logger** using the [Teensy 4.1](https://www.pjrc.com/store/teensy41.html).

---

## Why Teensy 4.1?

The **Teensy 4.1** was selected due to its **high analog bit depth** and **fast processing speed**, making it well-suited for data acquisition and logging applications.

### Feature Comparison: Teensy 4.1 vs Arduino Uno

| Feature            | Arduino Uno         | Teensy 4.1 (default specs)   |
|--------------------|---------------------|------------------------------|
| **Clock Speed**    | 16 MHz              | 600 MHz (can be overclocked) |
| **ADC Resolution** | 10-bit              | 12-bit                       |
| **DAC Capability** | None (PWM only)     | 2 DACs (MQS system)          |
| **Total Pins**     | 20 (14 Digital I/O, 6 Analog In) | Up to 55 (42 breadboard-friendly) |
| **Operating Voltage** | 5V              | 3.3V                         |

---

### I2C with Teensy 4.1

| Teensy Pin           | Sensor Pin        |
|--------------------|---------------------|
| **Pin 18**    | SDA              | 
| **Pin 19** | SCL              | 
| **3V3** | 3V3            | 
| **GND** | GND |

Note: Teensy 4.1 has two additional i2c buses available if needed, but sensors that don't share addresses can be chained. I'm using all sensors with QUIIC connections, double check sensor address to check for conflict. 
---

### Using Teensy with Arduino

Follow these instructions to use Teensy with arduino ide: https://www.pjrc.com/teensy/td_download.html

## Choosing an SD Card

For best performance with the Teensy 4.1 built-in microSD slot:

- **Capacity**: 32 GB (microSDHC) or 64 GB (microSDXC) is ideal  
- **Speed Class**: Look for UHS-I, Class 10, U3 or V30 rating (reliable sustained write speeds)  
- **Format**: Use **FAT32** (reformat SDXC cards if needed)  
- **Brands**: Stick to trusted options (SanDisk Extreme/Extreme Pro, Samsung EVO, Kingston Canvas)  

Larger or faster-rated cards (UHS-II, SD Express) will work but only at UHS-I speeds.

## Planned Features

-  Analog & digital data logging  
-  SD card storage  
-  Modular sensor inputs (temperature, light, etc.)  
-  Timestamped logs  

---

## Repository Structure

```plaintext
├── examples/         # Example logging sketches
├── docs/             # Documentation and diagrams
├── hardware/         # Wiring guides, schematics
└── README.md         # This file
