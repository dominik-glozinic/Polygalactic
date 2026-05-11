# Polygalactic

A handheld featuring an Asteroids-style game built entirely from scratch — custom firmware, custom PCB, and a 3D-printed shell.

![Status](https://img.shields.io/badge/status-in%20progress-yellow)

---
![Imgur](https://i.imgur.com/jBPUnnV.jpeg)

---

## Overview

The player controls a spaceship using two analog thumbsticks — the left stick moves the ship and the right stick aims and fires. Meteors spawn from the screen edges, vary in size and HP, and the game ends when the ship loses all three lives.

The goal of the project is to develop a finished handheld device. Currently uploaded is the 1st revision of the firmware that can be run on the dev device.

---
## Gameplay

https://github.com/user-attachments/assets/294df480-c689-42ea-9c88-54609ee2982e

---
## Hardware

| Component | Detail |
|---|---|
| MCU | STM32F103C8, ARM Cortex-M3 @ 72 MHz |
| Display | ILI9341, 320×240 TFT, 16-bit colour, SPI |
| Input | 2× analog thumbsticks via 12-bit ADC + DMA |
| Power | LiPo + USB-C charging (PCB, in progress) |

---

## Firmware highlights

- Semi-Custom ILI9341 driver (Some parts including the initialization sequence were borrowed from [afiskon/stm32-ili9341][u1])
- Only parts of the screen and objects are updated to keep up performance, reduce lag and prevent needless use of RAM
- Fixed-timestep game loop with frame-counter-based spawn rate to prevent timing issues under variable CPU load

---

## Project Status

| Phase | Status |
|---|---|
| Firmware |  Complete |
| PCB design |  In progress |
| CAD shell |  Planned |

---

## Technical Challenges

- Implemented a Partial Refresh algorithm to circumvent the 20KB SRAM limitation, enabling a 320x240 UI without a full frame buffer.
- Avoiding performance dips and lag by using DMA (Direct Memory Access)
- Writing a fast and reliable ILI9341 driver that doesn't overload the SPI connection by pushing all needed data at once instead of in chunks (also done to prevent tearing or freezing)

Current Challenges:
- Designing a PCB that removes all uneccessary parts from the dev board and only keeps needed ports
- Implementing a simple to use, reliable and secure SWD port for later debugging
- Integrating a USB-C Power Delivery circuit with dedicated Li-Ion protection and charging management

---

## Build

Firmware developed in STM32CubeIDE and CubeMX. Clone the repo, open the workspace, and flash via ST-Link over SWD.

---

## Author

Dominik Glozinic

[u1]: https://github.com/afiskon/stm32-ili9341/tree/master
