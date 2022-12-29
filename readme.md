# Makerfabs ESP32-S3 Parallel TFT with Touch

```c++
/*
Version:		V1.3
Author:			Vincent
Create Date:	2022/5/24
Note:
	2022/12/29	V1.3:Change wiki link.
	2022/10/7	V1.2:Add USB HID example.
	2022/10/7	V1.1:Add USB HID example.
*/
```
![](md_pic/main.gif)


[toc]

# Makerfabs

[Makerfabs home page](https://www.makerfabs.com/)

[Makerfabs Wiki](https://wiki.makerfabs.com/)

# Makerfabs ESP32-S3 Parallel TFT with Touch
## Intruduce

Product Link ：[ESP32-S3 Parallel TFT with Touch 3.5'' ILI9488](https://www.makerfabs.com/esp32-s3-parallel-tft-with-touch-ili9488.html)

Wiki Link : [ESP32-S3 Parallel 3.5'' TFT with Touch](https://wiki.makerfabs.com/ESP32_S3_Parallel_3.5_TFT_with_Touch.html)

The 3.5" 320x480 TFT LCD driver is ILI9488, it uses 16bits parallel line for communication with ESP32-S3, the main clock could be up to 20MHz, make the display smooth enough for videos; You can freely use some of Mabee pins(A I2c and a IOs) with the breakout connectors, to connect the ESP32-S3 display with sensors/ actuators, suitable for IoT applications.

![back](md_pic/back.jpg)

## Feature

- Wi-Fi (2.4 GHz band)
- Bluetooth Low Energy
- Dual high performance Xtensa® 32-bit LX7 CPU cores
- Ultra Low Power co-processor running either RISC-V or FSM core
- Multiple peripherals
- Built-in security hardware
- USB OTG interface
- USB Serial/JTAG Controller
- Arduino Compatible: You can play it with Arduino IDE
- LCD 3.5 inch Amorphous-TFT-LCD (Thin Film Transistor Liquid Crystal Display) for mobile-phone or handy electrical equipment
- LCD Driver: ILI9488(16bits parallel line)
- LCD Resolution: 320*480
- FT6236 Series ICs are single-chip capacitive touch panel controller IC with a built-in 16 bit enhanced Micro-controller unit (MCU)
- NS2009: A 4-wire resistive touch screen control circuit with I2C interface, which contains A 12-bit resolution A/D converter
- Power supply: 5V, Type-C USB
- Micro SD card slot on the board
- Dual USB Type-C: one for native USB and one for USB-to-UART
- Two Mabee interfaces
- Board size: 66mm * 85mm



# Code Explain

## Complier Option

- Install board : ESP32 .
- Install library : LovyanGFX library. 
- Edit the code based on the touch screen. If you use resistive screen, choice NS2009_TOUCH. If you use capacitive screen, choice FT6236_TOUCH. 
- Use type-c use cable connect USB-TTL to PC.
- Upload codes, select "ESP32-S3 DEV Module" and "UART0"

![](md_pic/complier.jpg)

## Firmware

### SD16_3.5

Factory firmware. First run the boot self-test to check the hardware. Press down the middle of the touch screen as prompted to display a LOGO picture on the SD card. Finally, print the touch coordinates in the serial port.



## Example

### touch_keyboard

The ESP32S3 native USB is used, which can be used as a USB HID to simulate a keyboard.

You can input text, key combination, enter key, and connect to the NTP server through WiFi, output real-time time.

 ### touch_keyboard V2.0

Added an inherited three custom keyboard keyboard Demo, including text input + key combination, numeric keypad, arrow keys + space.

 