# Edge-AI Wound Infection Detection Code

This folder contains the complete ESP32 firmware used in the project.

## Features

- Reads wound temperature using DS18B20
- Reads ambient temperature using DS18B20
- Reads humidity using DHT22
- Calculates temperature difference (ΔT)
- Applies moving average filtering
- Performs trend analysis
- Adaptive baseline learning
- Classifies wound condition
- Displays status on OLED
- Prints sensor values through Serial Monitor


## Required Libraries

- OneWire
- DallasTemperature
- DHT Sensor Library
- Adafruit GFX
- Adafruit SSD1306

Install all libraries through the Arduino IDE Library Manager.

## Upload Instructions

1. Open Arduino IDE.
2. Select ESP32 Development Board.
3. Install required libraries.
4. Connect ESP32.
5. Upload the code.
6. Open Serial Monitor (115200 baud).

## Expected Output

The ESP32 continuously displays:

- Wound Temperature
- Ambient Temperature
- Humidity
- ΔT
- Wound Status

Status values include:

- Normal
- Warning
- Critical
