<div align="center">

# 🔌 Programmable Controllers & Microcontrollers — UANL

![C](https://img.shields.io/badge/C-white?style=flat&logo=c&logoColor=white&labelColor=00bcd4&color=0097a7)
![AVR](https://img.shields.io/badge/AVR-white?style=flat&logo=arduino&logoColor=white&labelColor=00bcd4&color=0097a7)
![ATmega328P](https://img.shields.io/badge/ATmega328P-white?style=flat&logoColor=white&labelColor=00bcd4&color=0097a7)
![Proteus](https://img.shields.io/badge/Proteus-white?style=flat&logoColor=white&labelColor=00bcd4&color=0097a7)
![UANL](https://img.shields.io/badge/UANL-white?style=flat&logoColor=white&labelColor=00bcd4&color=0097a7)
![Status](https://img.shields.io/badge/Status-Complete-white?style=flat&logoColor=white&labelColor=00bcd4&color=0097a7)

> ⚠️ Code, comments and documentation are written in Spanish as part of my university coursework.

Projects and exercises from the Programmable Controllers & Microcontrollers course at UANL, using Proteus and Tinkercad for circuit simulation.

</div>

---

## 📋 About

This repository contains all the work done throughout the course, organized by activity. Every project targets the **ATmega328P** microcontroller and is programmed entirely in C at the register level, directly manipulating DDRx, PORTx, and peripheral registers without any Arduino framework.


---

## 📁 Contents

| # | Folder | Description |
|---|--------|-------------|
| 01 | **AFU04 — Internal Architecture** | LED light sequences and 7-segment display controlled by 2 push buttons; introduces I/O port manipulation, truth tables, and bit masking |
| 02 | **AFU01 — Midterm Exam (EMC)** | DC motor control via L293D driver + 4 LEDs + 7-segment display; 5 input combinations drive distinct LED patterns and motor states (stop, left, right, free-wheel, fast-stop) |
| 03 | **AFU05 — Programming Structures** | Countdown timer (59:59 → 00:00) with configurable minutes/seconds, multiplexed 4-digit display driven purely by timers, external interrupts for buttons, and a buzzer alarm |
| 04 | **AFU03 — PIA (Final Project)** | Intelligent greenhouse light control system: LDR sensor read via ADC, LED brightness adjusted by PWM, 4-digit multiplexed display showing 0–100 scale, FSM with IDLE / START / HOLD states |

---



## 🛠️ Built With

![C](https://img.shields.io/badge/C-white?style=flat&logo=c&logoColor=white&labelColor=00bcd4&color=0097a7)
![AVR-GCC](https://img.shields.io/badge/AVR--GCC-white?style=flat&logoColor=white&labelColor=00bcd4&color=0097a7)
![ATmega328P](https://img.shields.io/badge/ATmega328P-white?style=flat&logoColor=white&labelColor=00bcd4&color=0097a7)
![Proteus](https://img.shields.io/badge/Proteus-white?style=flat&logoColor=white&labelColor=00bcd4&color=0097a7)
![Tinkercad](https://img.shields.io/badge/Tinkercad-white?style=flat&logoColor=white&labelColor=00bcd4&color=0097a7)

---

## 📄 License

This project is licensed under the [MIT License](./LICENSE).
