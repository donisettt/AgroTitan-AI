# RoverScout - Smart Environment Monitoring Robot

> Prototype robot pemantau lingkungan berbasis ESP32 dengan kendali manual real-time via Web Dashboard HTTP.

![ESP32](https://img.shields.io/badge/ESP32-Arduino-blue)
![HTTP](https://img.shields.io/badge/Protocol-HTTP%20REST-green)
![DHT22](https://img.shields.io/badge/Sensor-DHT22%20%7C%20HC--SR04-orange)
![Status](https://img.shields.io/badge/Status-Prototype%20Active-brightgreen)

---

## Kelompok 6 - TIF RP 23 CID A

| Nama | NIM |
|------|-----|
| Doni Setiawan Wahyono | 23552011146 |
| Riki Gusti Fernanda | 23552011081 |
| Naufal Aulia Nuchrizal | 23552011366 |

---

## Deskripsi Project

**RoverScout** adalah prototype robot pemantau lingkungan berbasis ESP32 yang dapat dikendalikan secara remote melalui web dashboard. Robot ini dirancang untuk melakukan inspeksi kondisi lingkungan suatu area seperti gudang, ruangan server, atau area industri tanpa operator harus masuk secara fisik ke lokasi tersebut.

Rover dilengkapi sensor suhu dan kelembaban DHT22 untuk monitoring kondisi lingkungan secara real-time, sensor ultrasonik HC-SR04 untuk obstacle detection otomatis agar rover tidak menabrak objek di sekitarnya, serta sistem indikator LED dan buzzer sebagai peringatan lokal. Semua data sensor dan kontrol gerak rover tersedia pada dashboard web yang responsif dan dapat diakses dari perangkat mobile maupun desktop selama berada dalam jaringan WiFi yang sama.

## Dokumentasi Visual

### RoverScout V1

![RoverScout V1](assets/image/rover%20scout%20-%20v1.png)

### Wiring Diagram V1

![RoverScout Wiring Diagram V1](assets/wiring-diagrams/rover%20scout%20-%20wiring%20diagram%20v1.png)

### Latar Belakang

Monitoring kondisi lingkungan pada area tertentu umumnya masih dilakukan secara manual operator harus berjalan mengelilingi area untuk mengecek suhu, kelembaban, dan kondisi fisik sekitar. Pendekatan ini tidak efisien dan memiliki keterbatasan jangkauan. RoverScout hadir sebagai solusi prototype yang memungkinkan inspeksi area dilakukan secara remote menggunakan robot yang dikendalikan via browser tanpa instalasi aplikasi tambahan.

---

## Fitur Utama

| Fitur | Deskripsi |
|-------|-----------|
| **Kendali manual 4 arah** | Maju, mundur, kiri, kanan via web dashboard touch support untuk mobile |
| **Obstacle detection otomatis** | HC-SR04 mendeteksi objek di depan; rover berhenti otomatis dan buzzer aktif |
| **Monitoring suhu & kelembaban** | DHT22 membaca kondisi lingkungan real-time dan menampilkan di dashboard |
| **LED mundur otomatis** | GPIO26 menyala otomatis saat rover bergerak mundur |
| **Lampu penerangan depan** | GPIO27 dikontrol manual via dashboard untuk menerangi area gelap |
| **Klakson** | Buzzer dibunyikan manual dari dashboard sebagai peringatan area |
| **Pengaturan dinamis** | Kecepatan dan jarak stop sensor diatur langsung dari web tanpa re-upload |
| **System log real-time** | Semua aktivitas rover tercatat dengan timestamp di dashboard |
| **Remote restart** | ESP32 dapat di-restart via dashboard tanpa akses fisik ke perangkat |
| **Unduh log** | Log sistem diunduh sebagai file `.txt` untuk keperluan dokumentasi |
| **Responsive UI** | Dashboard adaptif untuk mobile dan desktop |

---

## Arsitektur Sistem

```
┌──────────────────────────────────────────────────────┐
│                  RoverScout (ESP32)                   │
│                                                      │
│   ┌─────────────┐   ┌─────────────┐                  │
│   │    DHT22    │   │   HC-SR04   │   Sensor Layer   │
│   │  Suhu & Hum │   │  Obstacle   │                  │
│   └──────┬──────┘   └──────┬──────┘                  │
│          │                 │                          │
│   ┌──────▼─────────────────▼──────┐                  │
│   │        ESP32 Core Logic        │                  │
│   │  WebServer + Motor Control +   │  Control Layer  │
│   │  Sensor Read + LED + Buzzer    │                  │
│   └──────┬─────────────────┬──────┘                  │
│          │                 │                          │
│   ┌──────▼──────┐   ┌──────▼──────┐                  │
│   │   L298N     │   │  LED/Buzzer │  Actuator Layer  │
│   │ Motor Driver│   │  Indicator  │                  │
│   └─────────────┘   └─────────────┘                  │
│                                                      │
│              WebServer HTTP Port 80                   │
└──────────────────────┬───────────────────────────────┘
                       │
                  WiFi (HTTP REST)
                       │
          ┌────────────▼───────────┐
          │     Web Dashboard      │
          │  Browser Mobile/Desktop│
          │  Monitoring + Control  │
          └────────────────────────┘
```

---

## Cara Kerja

```
Operator membuka dashboard di browser
          │
          ▼
Tekan tombol arah → HTTP request ke ESP32 → Motor bergerak
          │
          ▼
HC-SR04 scan tiap loop → Jika jarak < batas → Motor stop + Buzzer peringatan
          │
          ▼
DHT22 baca tiap 2 detik → Suhu & kelembaban update di dashboard
          │
          ▼
Dashboard polling /status tiap 1.5 detik → Semua data tampil real-time
```

---

## Pin Mapping

| Fungsi | Pin ESP32 | Keterangan |
|--------|-----------|------------|
| Motor IN1 | GPIO 18 | Arah motor kanan |
| Motor IN2 | GPIO 19 | Arah motor kanan |
| Motor IN3 | GPIO 16 | Arah motor kiri |
| Motor IN4 | GPIO 17 | Arah motor kiri |
| Motor ENA | GPIO 21 | PWM kecepatan motor kanan |
| Motor ENB | GPIO 22 | PWM kecepatan motor kiri |
| Ultrasonic TRIG | GPIO 4 | Trigger HC-SR04 |
| Ultrasonic ECHO | GPIO 5 | Echo HC-SR04 |
| Buzzer | GPIO 25 | Peringatan obstacle + klakson |
| DHT22 Data | GPIO 14 | Sensor suhu & kelembaban |
| LED Mundur | GPIO 26 | Nyala otomatis saat mundur |
| LED Lampu Depan | GPIO 27 | Dikontrol manual via dashboard |

---

## Tech Stack

| Area | Teknologi |
|------|-----------|
| Firmware | Arduino Framework (ESP32) |
| Microcontroller | ESP32 DevKit |
| Motor Driver | L298N Dual H-Bridge |
| Sensor | DHT22 (suhu/kelembaban), HC-SR04 (obstacle detection) |
| Aktuator | LED mundur, LED lampu depan, Buzzer |
| Komunikasi | HTTP REST WebServer ESP32 built-in |
| Dashboard | HTML + CSS + JavaScript (embedded dalam firmware) |
| Power | Baterai 18650 2S + Step-down ke ESP32 |

---

## API Endpoint

| Endpoint | Method | Fungsi |
|----------|--------|--------|
| `/` | GET | Halaman dashboard utama |
| `/status` | GET | JSON status lengkap rover |
| `/forward` | GET | Gerak maju |
| `/backward` | GET | Gerak mundur |
| `/left` | GET | Belok kiri |
| `/right` | GET | Belok kanan |
| `/stop` | GET | Berhenti |
| `/emergencystop` | GET | Emergency stop |
| `/lampu/on` | GET | Nyalakan lampu depan |
| `/lampu/off` | GET | Matikan lampu depan |
| `/klakson` | GET | Bunyikan klakson |
| `/settings` | GET | Update parameter (`?speed=&distLimit=`) |
| `/restart` | GET | Restart ESP32 remote |

### Contoh Response `/status`

```json
{
  "distance": 45.2,
  "status": "CLEAR",
  "uptime": 1523,
  "rssi": -52,
  "command": "STOP",
  "speed": 120,
  "distLimit": 30,
  "ip": "10.124.253.92",
  "temp": 26.8,
  "humidity": 73.5,
  "lampu": false
}
```

---

## Cara Penggunaan

### 1. Instalasi Library Arduino IDE

Pastikan library berikut sudah terinstall di Arduino IDE:

```
DHT sensor library by Adafruit
Adafruit Unified Sensor
WiFi bawaan ESP32
WebServer bawaan ESP32
```

### 2. Konfigurasi WiFi

Sesuaikan SSID dan password hotspot di bagian atas firmware:

```cpp
const char* ssid     = "nama_hotspot_anda";
const char* password = "password_anda";
```

### 3. Upload Firmware

Upload `main.ino` ke ESP32 menggunakan Arduino IDE. Setelah upload selesai, buka Serial Monitor (baud rate 115200) untuk melihat IP address yang diperoleh ESP32.

### 4. Akses Dashboard

Pastikan HP atau laptop terhubung ke WiFi yang sama dengan ESP32, lalu buka browser dan ketik IP yang tampil di Serial Monitor:

```
http://10.xxx.xxx.xxx
```

### 5. Operasikan Rover

- Tekan dan **tahan** tombol arah untuk menggerakkan rover
- **Lepas** tombol untuk berhenti
- Gunakan slider **Kecepatan** untuk mengatur PWM motor
- Gunakan slider **Jarak Stop** untuk mengatur sensitivitas obstacle detection
- Tombol **Lampu** untuk menyalakan/mematikan LED depan
- Tombol **Klakson** untuk membunyikan buzzer

---

## Struktur Repository

```
RoverScout/
├── firmware/
│   └── rover-scout/
│       └── main.ino              # Firmware utama ESP32
├── assets/
│   ├── image/
│   │   └── rover scout - v1.png
│   └── wiring-diagrams/
│       └── rover scout - wiring diagram v1.png
├── docs/
│   ├── Project Plan_Kelompok 6_RoverScout.docx
│   └── Project Plan_Kelompok 6_RoverScout.pdf
└── README.md
```

---

## Testing Checklist

| Area | Kriteria Berhasil |
|------|-------------------|
| Koneksi WiFi | ESP32 terhubung dan IP tampil di Serial Monitor |
| Akses dashboard | Browser membuka dashboard via IP ESP32 |
| Gerak maju | Rover maju saat tombol ditekan, berhenti saat dilepas |
| Gerak mundur | Rover mundur, LED GPIO26 menyala, buzzer beep lambat |
| Belok kiri | Rover berbelok ke kiri |
| Belok kanan | Rover berbelok ke kanan |
| Obstacle detection | Rover berhenti otomatis dan buzzer bunyi saat objek < distLimit |
| DHT22 | Suhu dan kelembaban terbaca dan tampil di dashboard |
| LED lampu depan | GPIO27 menyala/mati via tombol dashboard |
| Klakson | Buzzer berbunyi saat tombol klakson ditekan |
| Slider kecepatan | Kecepatan rover berubah sesuai nilai slider |
| Slider jarak stop | Batas obstacle berubah sesuai slider |
| System log | Aktivitas tercatat real-time di dashboard |
| Unduh log | File `.txt` berhasil diunduh dari browser |
| Remote restart | ESP32 restart dan dashboard reload otomatis |

---

## Batasan Prototype

- Rover dikendalikan secara manual belum ada navigasi otomatis
- Koneksi harus dalam jaringan WiFi lokal yang sama antara pengontrol dan ESP32
- ESP32-CAM belum terintegrasi pada versi ini; slot kamera sudah tersedia di dashboard
- Pembacaan tegangan baterai memerlukan voltage divider eksternal ke pin ADC
- Jangkauan kendali terbatas oleh kekuatan sinyal WiFi

---

## Risiko dan Mitigasi

| Risiko | Mitigasi |
|--------|----------|
| Koneksi WiFi tidak stabil | Gunakan hotspot HP yang sama; pastikan fitur AP Isolation dimatikan |
| Baterai cepat habis | Pisahkan rail power motor (L298N) dan ESP32 (step-down); gunakan 18650 kapasitas tinggi |
| Obstacle false positive | Kalibrasi nilai `distLimit` via slider di dashboard sesuai kondisi lapangan |
| Rover tidak merespons | Cek Serial Monitor; pastikan tidak ada blocking di `loop()` |
| LED/Buzzer tidak aktif | Verifikasi wiring GPIO dan `pinMode OUTPUT` di `setup()` |
| Dashboard tidak bisa diakses | Pastikan HP dan ESP32 di jaringan WiFi yang sama; cek IP di Serial Monitor |

---

## Potensi Pengembangan

- Integrasi **ESP32-CAM** untuk live video streaming area yang diinspeksi
- Penambahan **sensor kualitas udara** (MQ-135) untuk deteksi gas berbahaya
- Implementasi **FreeRTOS** dual-core untuk pemisahan task sensor dan motor control
- Penambahan **mode otomatis** berbasis line follower untuk patroli terjadwal
- Integrasi **MQTT** untuk pengiriman telemetri ke server monitoring eksternal
- Penambahan **notifikasi** saat suhu/kelembaban melewati ambang batas tertentu

---

## Lisensi

Universitas Teknologi Bandung  
Kelompok 6 - TIF RP 23 CID A  
Mata Kuliah Sistem Mikrokontroler