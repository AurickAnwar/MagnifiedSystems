# Real-Time Impact Severity Detection (IMU + ML)

Developing a real-time impact severity detection system using IMU sensor data and machine learning — with a helmet-mounted prototype for real-world testing.

**Organization:** [Magnified Systems](https://github.com/AurickAnwar/MagnifiedSystems)

---

## Overview

This project combines **embedded sensing** and **machine learning** to estimate how severe an impact is on a **1–100 scale**. An ESP32 reads motion data from an IMU; features from that data feed a model trained to predict severity. The long-term goal is a compact, helmet-mounted unit that can flag high-severity events in real time.

---

## Goals

- Capture **real-time motion** from an IMU on **ESP32**
- Build an **ML model** that predicts impact severity (**1–100**)
- Prototype a **helmet-mounted** sensor package for field tests
- Bridge firmware → data → training → inference pipeline

---

## System Architecture

```
┌─────────────────┐     ┌──────────────────┐     ┌─────────────────────┐
│  IMU + ESP32    │ ──► │  Feature extract │ ──► │  Severity model     │
│  (real-time)    │     │  (accel/gyro/    │     │  (1–100 prediction) │
│                 │     │   jerk, peaks)   │     │                     │
└─────────────────┘     └──────────────────┘     └─────────────────────┘
        │                                                    │
        └──────────── Helmet-mounted prototype ──────────────┘
```

| Layer | Role |
|--------|------|
| **Firmware** | Sample IMU, timestamp readings, stream or log data |
| **Dataset** | Labelled crashes / impacts with severity scores |
| **ML** | Train regressor (PyTorch) on sensor-derived features |
| **Deployment** | Run inference on-device or edge (future) |

---

## Hardware

- **MCU:** ESP32 (PlatformIO, `esp32doit-devkit-v1`)
- **Sensor:** IMU (accelerometer + gyroscope) for impact dynamics
- **Form factor:** Helmet-mounted prototype (in progress)

---

## Software Stack

| Component | Tools |
|-----------|--------|
| Embedded | PlatformIO, Arduino framework, C++ |
| ML | Python, Pandas, PyTorch |
| Data | CSV datasets (`crash_data_plausible.csv`) |

---

## Machine Learning (severity model)

A feedforward neural network learns to map crash-related **input features** to a **severity score**.

**Model (summary)**

- Input: 50 feature columns  
- Hidden: 32 → 16 (ReLU)  
- Output: 1 neuron (severity)  
- Loss: MSE · Optimizer: Adam · LR: `0.001` · Epochs: `1000`

**Dataset**

- File: `crash_data_plausible.csv`  
- 50 feature columns + `severity` target  

**Run training (Python)**

```bash
pip install pandas torch
python main.py
```

Training prints loss every 50 epochs and compares a sample **predicted vs actual** severity.

---

## Firmware (ESP32)

Firmware lives under `src/` (e.g. `main.cpp`). Typical flow:

1. Initialize IMU and serial  
2. Read accel/gyro at fixed rate  
3. Detect impact-like transients  
4. Log or transmit feature windows for ML  

Build and upload with **PlatformIO** (VS Code extension or CLI):

```bash
pio run
pio run -t upload
pio device monitor
```

---

## Project Structure

```
IMU sensor data/
├── src/                 # ESP32 firmware (main.cpp, datasets)
├── include/
├── lib/
├── test/
├── platformio.ini       # Board & dependencies
└── README.md
```

---

## Status

| Area | Status |
|------|--------|
| IMU + ESP32 data collection | In progress |
| ML severity model (offline) | Prototype trained on sample CSV |
| Helmet-mounted hardware | Design / testing |
| Real-time on-device inference | Planned |

---

## Roadmap

- [ ] Stable high-rate IMU logging on ESP32  
- [ ] Train/test split and proper evaluation metrics (MAE, R²)  
- [ ] Feature normalization and saved model weights  
- [ ] Label real helmet test impacts  
- [ ] Real-time severity estimate on ESP32 or companion module  
- [ ] Field tests with helmet prototype  

---

## Limitations (current)

- Sample / plausible training data — not yet full field-collected corpus  
- No production-grade train/validation split in early scripts  
- Real-time embedded inference not fully integrated yet  

---

## Author

**Aurick Anwar** — Mechatronics @ McMaster · Founding Engineer @ Magnified Systems

---

## License

Add a license file (e.g. MIT) if you plan to open-source this repo.
