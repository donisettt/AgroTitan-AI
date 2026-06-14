# Rover Scout Firmware

Firmware ini digunakan untuk mengontrol mobil robot AgroTitan-AI menggunakan ESP32-S NodeMCU dan driver motor L298N.

## Status Pengujian

✅ Motor DC berhasil dikontrol  
✅ Driver L298N berhasil digunakan  
✅ Kontrol maju, mundur, kiri, kanan, dan stop berhasil  
✅ Kontrol berbasis web melalui WiFi Access Point ESP32 berhasil  
✅ Kecepatan dan durasi gerak dapat diatur melalui halaman web  

## Hardware

- ESP32-S NodeMCU
- L298N Motor Driver
- 2x Motor DC Gearbox
- 2x Roda Chassis
- 2x Baterai 18650
- Saklar 2 pin

## Pin Mapping

| ESP32 | L298N |
|---|---|
| GPIO18 | IN1 |
| GPIO19 | IN2 |
| GPIO16 | IN3 |
| GPIO17 | IN4 |
| GPIO21 | ENA |
| GPIO22 | ENB |
| GND | GND |

## Power

| Sumber | Tujuan |
|---|---|
| Baterai + | 12V L298N |
| Baterai - | GND L298N |
| 5V L298N | 5V ESP32 |
| GND L298N | GND ESP32 |

## Web Control

ESP32 membuat WiFi Access Point:

```txt
SSID     : AgroTitan-Rover
Password : 12345678
URL      : http://192.168.4.1