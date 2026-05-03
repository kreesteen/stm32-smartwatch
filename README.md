# STM32 Embedded Smartwatch & Interactive System

## Overview
This project is a multi-feature embedded system built on an STM32 microcontroller as part of a university engineering design course. It implements a modular smartwatch-style interface with real-time embedded applications including sensor-based tracking, state machines, and interactive games.

My primary contributions focused on:
- Screen navigation and modular UI framework
- Step counter system
- Sleep tracking state machine
- Guided exercise routine system
- Fall detection simulation module

The system is written in C using STM32 HAL libraries and is designed around interrupt-style input handling and non-blocking timing using `HAL_GetTick()`.

---

## Key Features

### Home Screen (Real-Time Clock Simulation)
- Software-based real-time clock using system tick counter
- Time updates without blocking delays
- LCD-based UI with formatted date and time display

---

### Step Counter
- GPIO button-based step detection
- Persistent state tracking across screen updates
- Optimised LCD refresh (updates only on state change)

---

### Heart Rate Monitor (Simulation)
- Button-driven BPM increment simulation
- Efficient display updates with change detection logic

---

### Weather Display System
- Multi-location weather dataset using structs
- Button-based location cycling
- Custom LCD character for degree symbol rendering

---

### Sleep Tracker (State Machine Design)
- Inactivity-based sleep detection using timing thresholds
- Movement-based wake detection with event counters
- Multi-state logic:
  - Awake → Sleep → Wake detection → Result display
- Tracks total sleep duration and formats output (hh:mm:ss)

---

### Flashlight (Brightness Control)
- Multi-level brightness control using indexed array values
- Cyclic button input navigation
- Real-time LCD feedback

---

### Guided Exercise Routine
- Multi-step timed exercise system
- State machine controlling exercise and rest cycles
- Automatic progression through routine steps
- Completion handling and reset logic

---

### Fall Detection Simulation
- Event-driven alert system using GPIO input
- Countdown-based emergency simulation
- State machine:
  - Idle → Alert → Countdown → Emergency triggered/cancelled
- LED + LCD synchronised alert behaviour
  
---


### Embedded Mini Games

#### Coin Flip Simulator
- Random outcome generation using `rand()`
- Animated LCD transition effect

#### Magic 8 Ball
- Probability-weighted response selection
- Categorised responses (positive / neutral / negative)

#### Score Tracker
- Two-player scoring system
- Short press vs long press input detection
- Persistent state tracking and display updates

#### Reaction Time Game
- Randomised delay generation for unpredictability
- Measures reaction time using system tick counter
- State machine design:
  - READY → WAITING → GO → RESULT
- Performance-based feedback system

---

## Engineering Design Highlights

- Modular architecture with independent screen modules
- State machine-based system design across features
- Non-blocking timing using `HAL_GetTick()`
- GPIO input handling with debouncing logic
- Efficient LCD update strategy (minimising redraws)

---

## Technologies Used
- STM32 Microcontroller
- C (Embedded Systems Programming)
- STM32CubeIDE
- STM32 HAL Library
- GPIO, Timers, LCD interfacing

---

## Key Concepts Demonstrated
- Embedded state machines
- Real-time system design
- Event-driven programming
- Hardware–software integration
- Embedded UI design constraints



---

## 👤 Author
Kristine Bito-on  
Computer Engineering – UNSW  
GitHub: https://github.com/yourusername
