# Volvo-P3-SWM-ESP32-BLE-MediaControl

Control your music with your steering wheel buttons in your P3 Volvo.

## Requirements

- ESP32
- 12V to 5V/3.3V converter for powering the ESP32
- LIN-to-UART transceiver (I used a TJA1020 module)
- Torx 25 screwdriver and ratchet for removing the center console
- Wire for LIN, 12V and GND from the ICM (Infotainment Control Module / The Radio) to the ESP32
- Wire from the TJA1020 to the ESP32

## Installation

1. Remove the center console. Good video on how to: https://www.youtube.com/watch?v=p2Y7JbShlTw
2. Open the ICM
3. Solder your LIN wire to the pin corresponding to the Orange-Green wire (see picture) ![ICM Connector](/Pictures/IMG_0831.jpeg)
![ICM Cables](/Pictures/IMG_0830.jpeg)

![LIN-pin](/Pictures/IMG_0832.jpeg)

4. Find a 12V switched source and wire it to the 12V to 5V/3.3V converter

5. Flash your ESP32 with `LIN_to_Bluetooth.ino`

6. Connect to the Bluetooth device named `Volvo SWM BT`

## Issues

I had some issues with the LIN baud rate, to find the baud rate in your car. Upload the `LIN_baud_detection.ino` to your ESP32 instead of `LIN_to_Bluetooth.ino` and edit the `LIN_BAUD` in `LIN_to_Bluetooth.ino`. (Remember that the ICM must be on for LIN communication)