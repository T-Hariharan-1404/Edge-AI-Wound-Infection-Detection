# Edge-AI System for Early Wound Infection Detection

> A smart, low-cost, and non-invasive healthcare solution that uses Edge AI and sensor fusion for real-time wound infection monitoring.

---

## Overview

Wound infections can lead to delayed healing and serious complications if not detected early. Traditional monitoring methods often depend on manual inspection, making continuous observation difficult.

This project presents a **Context-Aware Edge-AI System** that continuously monitors wound conditions using temperature and humidity sensors. By comparing wound conditions with ambient environmental parameters, the system intelligently detects abnormal patterns and classifies the wound status as:

-  Normal
-  Warning
-  Critical

The entire analysis is performed locally on an **ESP32 microcontroller**, ensuring fast response, low power consumption, and enhanced data privacy.

---

##  Objectives

- Detect early signs of wound infection using non-invasive sensing.
- Monitor wound temperature and humidity in real time.
- Reduce false alarms through environmental compensation.
- Implement lightweight Edge-AI techniques for intelligent decision-making.
- Develop a scalable platform for future wearable healthcare devices.

---

# Key Features

- Real-time wound monitoring
- Context-aware analysis using ΔT and ΔH
- Edge-AI processing on ESP32
- Moving average filtering
- Trend analysis
- Adaptive baseline learning
- OLED-based status display
- Low-cost and energy-efficient design

---

## Hardware Components

| Component | Purpose |
|------------|----------|
| ESP32 | Main processing unit |
| DS18B20 (2x) | Wound & ambient temperature sensing |
| DHT22 | Humidity sensing |
| OLED Display | Real-time status display |
| Breadboard | Prototyping |
| Resistors & Jumper Wires | Circuit connections |

---

## Working Principle

1. Collect temperature and humidity data from sensors.
2. Calculate:
   - **ΔT = Wound Temperature − Ambient Temperature**
   - **ΔH = Humidity Variation**
3. Apply filtering and trend analysis.
4. Process data using Edge-AI logic.
5. Classify wound condition:
   -  Normal
   -  Warning
   -  Critical
6. Display results on OLED screen.

---

## Edge-AI Techniques

### Moving Average Filtering
Reduces sensor noise and improves reading stability.

### Trend Analysis
Identifies gradual increases in temperature and humidity that may indicate infection.

### Adaptive Baseline Learning
Learns normal wound conditions and adjusts thresholds dynamically.

### Context-Aware Decision Making
Compensates for environmental changes to improve detection accuracy.

---

##  Classification Criteria

| Status | Temperature Difference (ΔT) | Humidity |
|----------|--------------------------|----------|
|  Normal | < 1°C | < 60% |
|  Warning | 1°C – 2.5°C | 60% – 75% |
|  Critical | ≥ 2.5°C | ≥ 75% |

---

## 🔬 Results

The prototype was tested under simulated wound conditions.

###  Normal Condition
- Stable temperature
- Low humidity
- Correctly identified as Normal

###  Warning Condition
- Slight increase in temperature and moisture
- Correctly identified as Warning

###  Critical Condition
- Significant increase in temperature and humidity
- Correctly identified as Critical

### Observations
- Accurate classification across test scenarios
- Reduced false alarms through contextual analysis
- Stable and reliable sensor performance
- Real-time response without cloud dependency

---

##  Future Scope

- Mobile application integration
- Cloud-based health monitoring
- TinyML implementation
- Wearable smart bandage development
- Predictive analytics for infection progression
- Remote healthcare monitoring

---

 **This project demonstrates how Edge AI, IoT, and embedded systems can be combined to create practical healthcare solutions for the future.**
