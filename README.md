# EcoSort - Smart Trash Sorting Bin

Proyek mikrokontroler berbasis ESP32 yang mendeteksi dan memilah sampah secara otomatis menggunakan sensor ultrasonik dan sensor kelembaban, lalu menampilkan data secara real-time melalui dashboard web berbasis MQTT.

**Anggota Kelompok 1:**
- Lisa Ayu Aryanti - 23552011432
- Rafti Astia Rahayu - 23552011392
- Luki Solihin - 23552011413

---

## Struktur Folder

```
project_Mikrokontroller_kelompok1/
|
|-- firmware/
|   |-- ecosort.ino          # Seluruh kode ESP32: sensor, servo, WiFi, MQTT
|
|-- dashboard/
|   |-- dashboard.html       # Dashboard web lengkap (HTML + CSS + JS dalam satu file)
|
|-- README.md
```

---

## Komponen

### Hardware

| Komponen | Pin ESP32 | Fungsi |
|---|---|---|
| Sensor Ultrasonik HC-SR04 | TRIG: GPIO5, ECHO: GPIO18 | Mendeteksi keberadaan sampah (threshold < 15 cm) |
| Sensor Soil Moisture | GPIO34 (ADC) | Mengukur kelembaban untuk membedakan organik/anorganik |
| Servo Motor | GPIO13 | Memilah sampah ke kompartemen organik atau anorganik |
| ESP32 | - | Mikrokontroler utama, WiFi, dan komunikasi MQTT |

### Software / Library

| Library / Tool | Fungsi |
|---|---|
| `WiFi.h` | Koneksi ESP32 ke jaringan WiFi |
| `PubSubClient` | Komunikasi MQTT antara ESP32 dan dashboard |
| `ESP32Servo` | Kontrol servo motor via ESP32 |
| MQTT.js (CDN) | Koneksi MQTT WebSocket di sisi dashboard |

### Konfigurasi MQTT

| Parameter | Nilai |
|---|---|
| Broker | `broker.hivemq.com` |
| Port firmware (TCP) | `1883` |
| Port dashboard (WSS) | `8884` |
| Topik sensor | `ecosort/sensor` |
| Topik servo | `ecosort/servo` |
| Topik status | `ecosort/status` |

---

## Alur Code

### Firmware (`ecosort.ino`)

```
[Power ON / Boot]
     |
     v
[setup()]
  - Serial begin (115200 baud)
  - Konfigurasi pin: TRIG OUTPUT, ECHO INPUT
  - Servo attach ke GPIO13, posisi awal 90 (tengah)
  - Koneksi WiFi via setup_wifi()
      - Jika gagal setelah 40 percobaan -> ESP.restart()
  - Set MQTT server: broker.hivemq.com:1883
     |
     v
[loop()] -- berjalan terus-menerus
     |
     +-- Cek koneksi MQTT -> reconnect_mqtt() jika terputus
     |       - Publish status online ke ecosort/status saat reconnect
     |       - Jika gagal 10x -> ESP.restart()
     |
     +-- getDistance(): baca HC-SR04 (rata-rata 3 pengukuran)
     |
     +-- Jika jarak 1–15 cm (ada sampah):
     |       |
     |       +-- Delay 500ms, lalu getSoilHumidity()
     |               - Baca ADC 5x, rata-rata, map ke 0–100%
     |               - Raw ADC constrain antara 1500–4095
     |       |
     |       +-- Klasifikasi:
     |               humidity > 30% -> ORGANIK
     |               humidity <= 30% -> ANORGANIK
     |       |
     |       +-- Publish ke ecosort/sensor:
     |               {"distance": N, "humidity": N, "jenis": "organik/anorganik"}
     |       |
     |       +-- Gerakkan servo:
     |               ORGANIK   -> 170 derajat (kiri)
     |               ANORGANIK -> 10 derajat (kanan)
     |       |
     |       +-- Publish posisi servo ke ecosort/servo
     |       +-- Tahan posisi 3000ms (SERVO_HOLD_MS)
     |       +-- Kembali ke tengah (90 derajat), publish standby
     |
     +-- Jika jarak >= 15 cm: tidak ada sampah, lanjut
     |
     +-- Delay 1000ms -> ulangi loop
```

### Dashboard (`dashboard.html`)

```
[Browser membuka dashboard.html]
     |
     v
[HTML + CSS dimuat]
  - Tampilkan layout: status bar, kartu jenis sampah,
    kartu sensor, indikator servo, counter, log aktivitas
     |
     v
[JavaScript berjalan]
  - Buat koneksi MQTT via WebSocket (wss://broker.hivemq.com:8884/mqtt)
  - Client ID unik: "EcoSortDashboard-" + random hex
     |
     v
[Event: connect]
  - Status dot berubah hijau (animasi pulse)
  - Subscribe ke 3 topik: ecosort/sensor, ecosort/servo, ecosort/status
     |
     v
[Event: message]
     |
     +-- Topik ecosort/sensor:
     |     - Parse JSON: {distance, humidity, jenis}
     |     - Update kartu jarak dan kelembaban
     |     - Update kartu jenis sampah (warna + label berubah)
     |     - Increment counter organik atau anorganik
     |     - Tambahkan entri baru ke log aktivitas (max 30 item)
     |
     +-- Topik ecosort/servo:
           - Parse JSON: {posisi, derajat}
           - Geser indikator visual servo (kiri/tengah/kanan)
           - Update teks posisi servo
     |
     v
[Event: offline / error]
  - Status dot berubah merah
  - Tampilkan pesan error atau "mencoba kembali"
```

---

## Alur Demo Proyek

### 1. Persiapan Hardware

Rangkai komponen sesuai pin mapping berikut:

| Komponen | Pin ESP32 |
|---|---|
| HC-SR04 VCC | 3.3V |
| HC-SR04 GND | GND |
| HC-SR04 TRIG | GPIO5 |
| HC-SR04 ECHO | GPIO18 |
| Soil Moisture VCC | 3.3V |
| Soil Moisture GND | GND |
| Soil Moisture AOUT | GPIO34 |
| Servo Signal | GPIO13 |
| Servo VCC | 5V (VIN) |
| Servo GND | GND |

### 2. Konfigurasi dan Upload Firmware

Buka `firmware/ecosort.ino` di Arduino IDE, lalu ubah baris berikut sesuai jaringan yang digunakan:

```cpp
const char* WIFI_SSID     = "NAMA_WIFI_KAMU";
const char* WIFI_PASSWORD = "PASSWORD_WIFI_KAMU";
```

Install library yang diperlukan melalui Library Manager Arduino IDE:
- `PubSubClient` by Nick O'Leary
- `ESP32Servo` by Kevin Harrington

Pilih board **ESP32 Dev Module**, lalu klik Upload. Buka Serial Monitor (115200 baud) untuk memverifikasi koneksi WiFi dan MQTT berhasil.

Output Serial Monitor saat berhasil:
```
=================================
  EcoSort Smart Bin — Booting
=================================
[WiFi] Menghubungkan ke: NAMA_WIFI
[WiFi] Terhubung!
[WiFi] IP Address: 192.168.x.x
[MQTT] Menghubungkan... Terhubung!
[System] EcoSort siap!
```

### 3. Jalankan Dashboard

Buka file `dashboard/dashboard.html` langsung di browser (tidak memerlukan server lokal). Dashboard akan otomatis terhubung ke `broker.hivemq.com` via WebSocket dan mulai menerima data.

Indikator koneksi di bagian atas akan berubah hijau dan berdenyut saat berhasil terhubung.

### 4. Pengujian Sistem

Letakkan benda di depan sensor ultrasonik pada jarak kurang dari 15 cm. Sistem akan bekerja secara urutan:

1. Sensor ultrasonik mendeteksi objek.
2. Sensor kelembaban membaca nilai moisture dari objek.
3. ESP32 mengklasifikasikan: `humidity > 30%` = organik, selainnya = anorganik.
4. Data dikirim ke MQTT broker.
5. Servo bergerak ke posisi yang sesuai dan kembali ke tengah setelah 3 detik.
6. Dashboard memperbarui semua kartu secara real-time dan menambah entri di log.

### 5. Evaluasi

- Pantau Serial Monitor untuk melihat nilai jarak dan kelembaban mentah.
- Amati dashboard untuk memverifikasi data yang diterima sesuai dengan kondisi fisik.
- Uji dengan benda basah (kelembaban tinggi) dan benda kering (kelembaban rendah) untuk memverifikasi akurasi klasifikasi.
- Sesuaikan nilai `HUMIDITY_THRESH` (default: 30) di firmware jika diperlukan kalibrasi.
