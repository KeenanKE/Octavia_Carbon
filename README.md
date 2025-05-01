# RTD Input Module for ESP32 (Pt100/Pt1000)

## Overview
This firmware runs on an **ESP32 (30-pin dev board)** and interfaces with a precision analog front-end to read temperature from either a **Pt100 or Pt1000 RTD** using a 4-wire configuration. The analog voltage is amplified using an **INA333 instrumentation amplifier**, digitized by an **ADS1118 16-bit SPI ADC**, and displayed in real time on a **SSD1306 I2C OLED**.

The firmware includes a mechanism to detect **which RTD (Pt100 or Pt1000)** is selected via a hardware **SPDT switch** connected to a GPIO pin. Based on this, the code **adjusts scaling parameters accordingly**, fulfilling the optional bonus requirement in the Octavia Carbon I&C Technical Challenge.

---

## Hardware Features
- LM134H constant current source (~250 µA) for RTD excitation
- SPDT switches (SW1, SW2) for RTD selection
- INA333 amplifies voltage drop across RTD
- ADS1118 digitizes signal via SPI
- ESP32 reads temperature and drives OLED
- TVS diodes and RC filtering for robustness

---

## Bill of Materials
| Component     | Part                          |
|--------------|-------------------------------|
| MCU          | ESP32 Dev Module (30-pin)     |
| RTDs         | Pt100 and Pt1000 (4-wire)     |
| Amplifier    | INA333                        |
| ADC          | ADS1118 (16-bit SPI)          |
| Display      | SSD1306 OLED (128×64, I2C)     |
| Current Src. | LM134H + Resistor (R1)        |
| Filter       | R3 = 1 kΩ, C1 = 100 nF         |
| Selector     | SPDT switch (GPIO4 input)     |

---

## Pin Assignments (ESP32)
| Function         | GPIO    |
|------------------|---------|
| SPI SCLK         | GPIO18  |
| SPI MISO         | GPIO19  |
| SPI MOSI         | GPIO23  |
| SPI CS (ADS1118) | GPIO5   |
| I2C SDA (OLED)   | GPIO21  |
| I2C SCL (OLED)   | GPIO22  |
| RTD SELECT       | GPIO4   |

---

## What the Code Does

1. **Initializes**:
   - SPI (ADS1118)
   - I2C (OLED)
   - GPIO for RTD selection (Pt100/Pt1000)

2. **Reads ADC data** from ADS1118.

3. **Converts ADC value** to:
   - Voltage
   - Resistance
   - Temperature

4. **RTD Selection Logic**:
   - A SPDT switch connects to GPIO4
   - If GPIO4 is LOW → Pt100
   - If GPIO4 is HIGH → Pt1000
   - Code adjusts R₀ accordingly:
     ```cpp
     float R0 = isPt100 ? 100.0 : 1000.0;
     ```

5. **Displays values** on OLED:
   - RTD Type
   - Voltage (V)
   - Resistance (Ω)
   - Temperature (°C)

---

## Bonus Feature
✔️ Dynamic RTD scaling based on GPIO state satisfies the I&C Challenge optional bonus requirement.

---

## Requirements
- Arduino IDE or PlatformIO
- Libraries:
  - `Adafruit_SSD1306`
  - `Adafruit_GFX`

---

## Usage
1. Wire circuit according to schematic.
2. Set SPDT to select desired RTD.
3. Upload firmware to ESP32.
4. View live results on OLED display.

---

## Files Included
- `main.ino` — ESP32 firmware
- `README.md` — This documentation
