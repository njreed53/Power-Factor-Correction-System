# Power-Factor-Correction-System
> ⚠️ **Safety Notice:** This project involves 120V AC mains voltage. Do not replicate without proper training, equipment, and supervision.`

Table of Contents
=================
* [Power-Factor-Correction-System](#power-factor-correction-system)
   * [Introduction](#introduction)
   * [Problem Definition](#problem-definition)
   * [Engineering Requirements](#engineering-requirements)
   * [Engineering Requirements](#engineering-requirements-1)
      * [1. Voltage Measurement Block](#1-voltage-measurement-block)
      * [2. Current Measurement Block](#2-current-measurement-block)
      * [3. MCU Requirements](#3-mcu-requirements)
      * [4. Relay System](#4-relay-system)
      * [5. Capacitor Bank](#5-capacitor-bank)
      * [6. Power Supply](#6-power-supply)
   * [Firmware Design and Implementation](#firmware-design-and-implementation)
   * [Results](#results)
      * [Qualitative Performance Assessment](#qualitative-performance-assessment)
         * [Waveform Captures](#waveform-captures)
      * [Limitations and Uncompleted Testing](#limitations-and-uncompleted-testing)
   * [License](#license)
   * [Questions?](#questions)

## Introduction
Power factor correction (PFC) is an important part of maintaining efficiency and stability in commercial and industrial electrical systems. Inductive loads like motors, HVAC equipment, and pumps cause power factor to drop, which increases current draw and wastes energy. Automatic PFC (APFC) systems fix this by monitoring the phase difference between voltage and current waveforms and adding capacitance to bring the power factor near unity.

**Project Goal:** Design and build a portable, low-cost single-phase APFC module for small to mid-sized commercial facilities. The system measures voltage and current on a single line, calculates the power factor, and automatically switches in capacitors for correction.

## Problem Definition
Most modern PFC systems rely on accurate sensing of both voltage and current waveforms to calculate phase angle. Current transformers (CTs) and voltage transformers (VTs) are the standard sensing methods because they provide electrical isolation and output low-voltage AC signals safe to interface with measurement circuits. For voltage measurement, modules like the ZMPT101B offer isolated, scaled-down AC line waveforms suitable for ADC conversion.

Once stepped down, signals require conditioning before entering a microcontroller: buffering, low-pass filtering, and DC offset adjustment keep the entire AC waveform within the ADC's range.

**The Gap:** While industrial APFC cabinets are common, they're large, high-power systems built for factories. Smaller, portable APFC modules are under-researched. Existing hardware either focuses on basic monitoring or is too expensive and bulky for small to mid-sized commercial users.

Small businesses relying on three-phase power often experience low power factor from heavy inductive loads, which results in increased current draw, reduced efficiency, and potential utility penalties. There's a clear need for a modular, lower-cost, easy-to-install solution.

**Our Objective:** Build a single-phase APFC module that measures voltage and current, calculates power factor, and automatically switches in capacitors to correct to near unity. The module will be portable, cost-effective compared to existing solutions, easily installable, and suitable for small to mid-sized commercial facilities.

## Engineering Requirements

Full requirements specification with 49 technical requirements across all subsystems is available in the documentation:

📄 **[View Complete Requirements Specification](docs/requirements.pdf)**

## Engineering Requirements

Below is the complete requirements specification for the Power Factor Correction System. Requirements are organized by subsystem.

---

### 1. Voltage Measurement Block

| ID  | Requirement                                                                               | Justification                                                |
| --- | ----------------------------------------------------------------------------------------- | ------------------------------------------------------------ |
| 1.1 | Scale input voltage to ≤ 3.0 V peak at ADC input                                          | Prevents overvoltage damage to MCU ADC                       |
| 1.2 | ADC input signal shall remain within 0–3.3 V during operation                             | Ensures safe and valid ADC operation                         |
| 1.3 | Voltage divider shall maintain linearity with ≤ ±2% error                                 | Required for accurate voltage measurement and PF calculation |
| 1.4 | Voltage divider resistors shall have ≤ ±1% tolerance and operate below 50% of rated power | Improves accuracy and ensures safe operation                 |

---

### 2. Current Measurement Block

|ID|Requirement|Justification|
|---|---|---|
|2.1|Operate over 0–5 A RMS at 60 Hz|Matches expected load conditions|
|2.2|Current transformer shall not exceed 5 A RMS primary current|Prevents CT saturation and damage|
|2.3|Burden resistor shall produce ~1.0 V peak (±10%) at 5 A RMS|Provides a measurable signal for the ADC|
|2.4|Current measurement signal shall remain within 0–3.3 V after conditioning|Ensures compatibility with ADC input range|
|2.5|Current measurement accuracy shall be ≤ ±2% from 0.5–5 A RMS|Ensures reliable power calculations|
|2.6|Voltage and current signals shall be sampled synchronously at 10 kS/s ±1%|Required for accurate phase and PF calculation|
|2.7|System shall operate at room temperature (20–30°C)|Defines expected operating conditions|

---

### 3. MCU Requirements

|ID|Requirement|Justification|
|---|---|---|
|3.1|Operate at 3.3 V ±5% with load current up to 60 mA|Ensures stable microcontroller operation|
|3.2|ADC shall provide 12-bit resolution and sample at 10 kS/s ±1%|Enables accurate signal acquisition|
|3.3|RMS measurement error shall be ≤ ±3% for 0.2–2.5 V RMS inputs|Ensures acceptable measurement accuracy|
|3.4|Transmit data at 115200 baud ±2%|Provides reliable communication|
|3.5|Update RMS and PF calculations at least every 50 ms|Ensures timely system response|

---

### 4. Relay System

|ID|Requirement|Justification|
|---|---|---|
|4.1|Switch capacitor banks under MCU control without introducing voltage spikes|Prevents damage and ensures stable operation|
|4.2|Support up to 8 channels|Allows future system expansion|
|4.3|Each relay shall be rated for ≥ 120 V AC and ≥ 10 A|Ensures safe operation under load|
|4.4|Relays shall default to normally open (NO) configuration|Ensures capacitors are disconnected by default for safety|
|4.5|Relay switching shall occur near voltage zero-crossing|Minimizes inrush current and switching transients|
|4.6|Provide electrical isolation from the MCU|Protects low-voltage circuitry|
|4.7|Disconnect the system under fault conditions|Provides safety shutdown capability|
|4.8|Operate reliably from 0–40°C|Defines operating environment|
|4.9|Inrush current during switching shall not exceed capacitor ratings|Prevents capacitor damage|

---

### 5. Capacitor Bank

|ID|Requirement|Justification|
|---|---|---|
|5.1|Provide reactive power for PF correction|Core system functionality|
|5.2|Include at least three discrete capacitors|Enables step-based correction|
|5.3|Capacitors shall be rated for ≥ 120 V AC|Ensures safe operation on mains voltage|
|5.4|Arrange capacitors in discrete steps (e.g., 1 μF, 2 μF, 4 μF)|Allows flexible PF adjustment|
|5.5|Achieve PF ≥ 0.95 (±5%) for selected load|Meets performance target|
|5.6|Each capacitor shall include a parallel bleeder resistor|Ensures safe discharge when disconnected|
|5.7|Switch via MCU-controlled relays|Enables automated correction|
|5.8|Operate continuously under expected load conditions|Ensures reliability|
|5.9|Include a fuse to protect capacitor bank from fault conditions|Provides overcurrent protection|

---

### 6. Power Supply

|ID|Requirement|Justification|
|---|---|---|
|6.1|Wall adapter shall operate from 120 V AC ±10% at 60 Hz|Ensures compatibility with standard mains supply|
|6.2|PT shall provide 12 V AC ±10% under nominal load|Supplies required voltage for downstream circuitry|
|6.3|PT shall include a 3.5 A slow-blow fuse for protection|Limits fault current and protects the system|
|6.4|AC-DC converter shall accept input range of 85–305 V AC|Allows robust operation under varying input conditions|
|6.5|AC-DC converter shall provide 5.0 V DC ±5% up to 500 mA|Ensures stable power for MCU and peripherals|
|6.6|5 V output ripple shall not exceed 100 mV peak-to-peak|Maintains signal integrity and stable operation|
|6.7|All low-voltage circuitry shall operate within rated supply limits during continuous operation|Prevents malfunction or damage to components|
## Firmware Design and Implementation

[Insert code flowchart here]

The firmware for the system is implemented on an STM32-F303K8 microcontroller and handles data acquisition, signal processing, power factor calculation, and capacitor bank control. Source files are located in `/firmware/` repository directory.

At startup, the system initializes all peripherals: ADC with DMA, timers, GPIO pins for relay control, UART for debugging, and I2C for the LCD interface. Once initialized, the ADC starts in DMA mode and continuously collects voltage and current samples triggered by a timer.

When the ADC buffer fills, the main processing routine executes:

1. DC offset removal from sampled signals
2. RMS voltage and current computation
3. Real and apparent power calculation
4. Instantaneous power factor determination

A simple moving average filter stabilizes the power factor reading. Protection logic disables the capacitor bank and triggers a fault state if voltage or current levels fall outside acceptable ranges.

The control algorithm compares measured power factor to a 0.95 target. If outside the defined deadband, required reactive power compensation is calculated. The controller determines the appropriate capacitor combination using a binary-based relay control scheme.

Relay signals update to adjust the capacitor bank, and system status (voltage, current, power factor) transmits over UART for monitoring. The process repeats continuously.
## Results

The primary outcome was successful assembly and operation of a single-phase Automatic Power Factor Correction (APFC) prototype. While comprehensive quantitative testing was not completed due to timeline constraints, the system demonstrated core functionality during demonstration and informal testing.

---
### Qualitative Performance Assessment

During informal testing with an inductive motor load, the following behaviors were observed:

|Observation|Result|
|---|---|
|**Correction Behavior**|MCU activation reduced phase difference between voltage and current on oscilloscope, indicating improved power factor|
|**System Stability**|Prototype operated continuously without fault conditions or relay chatter|
|**Display Output**|Terminal displayed correct feedback on system status and capacitor bank configuration|

#### Waveform Captures

_[Insert images here]_

> **Figure 1:** Voltage (top) and Current (bottom) from probing motor directly  
> **Figure 2:** Voltage waveform after step-down, DC-bias, and RC filter  
> **Figure 3:** Current waveform after DC-bias and RC filter  
> **Figure 4:** Terminal output displaying automatic switching and voltage/current/PF readings

---

### Limitations and Uncompleted Testing

Comprehensive quantitative testing was limited by project timeline constraints. The following test plans from Section 11 were not fully executed:

- Voltage divider ratio verification with calibrated equipment
- Current measurement accuracy across full 0.5–5 A range
- ADC sampling rate verification with oscilloscope
- Relay in-rush current measurement
- Formal PF correction verification with multiple load conditions
- Power supply ripple and isolation testing

## License

This project is licensed under the MIT License — see [`LICENSE`](LICENSE) file for details.

# Questions?

Open an issue or reach out directly. Happy to discuss the implementation!
