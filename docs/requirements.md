
## 1. Voltage Measurement Block

| ID  | Engineering Requirement                                                                    | Justification                                                 |
| --- | ------------------------------------------------------------------------------------------ | ------------------------------------------------------------- |
| 1.1 | The voltage measurement block shall scale input voltage to ≤ 3.0 V peak at the ADC input.  | Prevents overvoltage damage to the MCU ADC.                   |
| 1.2 | The ADC input signal shall remain within 0–3.3 V during operation.                         | Ensures safe and valid ADC operation.                         |
| 1.3 | The voltage divider shall maintain linearity with ≤ ±2% error.                             | Required for accurate voltage measurement and PF calculation. |
| 1.4 | Voltage divider resistors shall have ≤ ±1% tolerance and operate below 50% of rated power. | Improves accuracy and ensures safe operation.                 |

---

## 2. Current Measurement Block

|ID|Engineering Requirement|Justification|
|---|---|---|
|2.1|The current measurement block shall operate over 0–5 A RMS at 60 Hz.|Matches expected load conditions.|
|2.2|The current transformer shall not exceed 5 A RMS primary current.|Prevents CT saturation and damage.|
|2.3|The burden resistor shall produce ~1.0 V peak (±10%) at 5 A RMS.|Provides a measurable signal for the ADC.|
|2.4|The current measurement signal shall remain within 0–3.3 V after conditioning.|Ensures compatibility with ADC input range.|
|2.5|Current measurement accuracy shall be ≤ ±2% from 0.5–5 A RMS.|Ensures reliable power calculations.|
|2.6|Voltage and current signals shall be sampled synchronously at 10 kS/s ±1%.|Required for accurate phase and PF calculation.|
|2.7|The system shall operate at room temperature (20–30°C).|Defines expected operating conditions.|

---

## 3. Microcontroller & ADC

|ID|Engineering Requirement|Justification|
|---|---|---|
|3.1|The MCU shall operate at 3.3 V ±5% with load current up to 60 mA.|—|
|3.2|The ADC shall provide 12-bit resolution and sample at 10 kS/s ±1%.|Enables accurate signal acquisition.|
|3.3|RMS measurement error shall be ≤ ±3% for 0.2–2.5 V RMS inputs.|Ensures acceptable measurement accuracy.|
|3.4|The MCU shall transmit data at 115200 baud ±2%.|Provides reliable communication.|
|3.5|The MCU shall update RMS and PF calculations at least every 50 ms.|Ensures timely system response.|

---

## 4. Relay Block

|ID|Engineering Requirement|Justification|
|---|---|---|
|4.1|The relay block shall switch capacitor banks under MCU control without introducing voltage spikes.|Prevents damage and ensures stable operation.|
|4.2|The relay system shall support up to 8 channels.|Allows future system expansion.|
|4.3|Each relay shall be rated for ≥ 120 V AC and ≥ 10 A.|Ensures safe operation under load.|
|4.4|Relays shall default to normally open (NO) configuration.|Ensures capacitors are disconnected by default for safety.|
|4.5|Relay switching shall occur near voltage zero-crossing.|Minimizes inrush current and switching transients.|
|4.6|The relay block shall provide electrical isolation from the MCU.|Protects low-voltage circuitry.|
|4.7|A relay shall disconnect the system under fault conditions.|Provides safety shutdown capability.|
|4.8|Relays shall operate reliably from 0–40°C.|Defines operating environment.|
|4.9|Inrush current during switching shall not exceed capacitor ratings.|Prevents component damage.|

---

## 5. Capacitor Bank

|ID|Engineering Requirement|Justification|
|---|---|---|
|5.1|The capacitor bank shall provide reactive power for PF correction.|Core system functionality.|
|5.2|The capacitor bank shall include at least three discrete capacitors.|Enables step-based correction.|
|5.3|Capacitors shall be rated for ≥ 120 V AC.|Ensures safe operation on mains voltage.|
|5.4|Capacitors shall be arranged in discrete steps (e.g., 1 μF, 2 μF, 4 μF).|Allows flexible PF adjustment.|
|5.5|The system shall achieve PF ≥ 0.95 (±5%) for the selected load.|Meets performance target.|
|5.6|Each capacitor shall include a parallel bleeder resistor.|Ensures safe discharge when disconnected.|
|5.7|Capacitors shall be switched via MCU-controlled relays.|Enables automated correction.|
|5.8|The capacitor bank shall operate continuously under expected load conditions.|Ensures reliability.|
|5.9|A fuse shall protect the capacitor bank from fault conditions.|Provides overcurrent protection.|

---

## 6. Power Supply

|ID|Engineering Requirement|Justification|
|---|---|---|
|6.1|The PT wall adapter shall operate from 120 V AC ±10% at 60 Hz.|Ensures compatibility with standard mains supply.|
|6.2|The PT shall provide 12 V AC ±10% under nominal load.|Supplies required voltage for downstream circuitry.|
|6.3|The PT shall include a 3.5 A slow-blow fuse for protection.|Limits fault current and protects the system.|
|6.4|The AC–DC converter shall accept an input range of 85–305 V AC.|Allows robust operation under varying input conditions.|
|6.5|The AC–DC converter shall provide 5.0 V DC ±5% up to 500 mA.|Ensures stable power for MCU and peripherals.|
|6.6|The 5 V output ripple shall not exceed 100 mV peak-to-peak.|Maintains signal integrity and stable operation.|
|6.7|All low-voltage circuitry shall operate within rated supply limits during continuous operation.|Prevents malfunction or damage to components.|
