ESP32 C3 Super Mini Mochi Bot
Fitur:
1. Intro & Idle Animation
2. Battery Indikator, Charging Indikator, & Full Charing Indikator
3. One tap - Blinking Animation
4. Double tap - Random Animation
5. Triple tap - Love Animation
6. Long Press - UwU Animation
7. Long Press > 3s - Play Video(anda dapat mengubah video yang diputar dengan membuat sendiri animasinya)

Untuk kostumisasi animasi longpress > 3s saya sarankan menggunakan salah satu dari tools berikut:
https://triwahyu45.github.io/ESP32-OLED-Video-Converter/
https://animator.wokwi.com/
https://www.oledanimationmaker.com/

Koneksi esp32 ke Oled lcd
3.3v -> vcc
gnd -> gnd
8 -> sda
9 -> scl

esp32 ke tp223
3.3v -> vcc
4 -> out
gnd -> gnd

untuk battery manager
esp32 ke tp4056
5v -> out+
gnd -> out-
2 -> 7 (chrg)
1 -> 6 (stdby)

esp32 ke resistor 10k/10k (divider)
5v<-=10k=||=10k=->gnd
         ||
         3

Flashing:
saya asumsikan anda benar-benar pernah melakukan flash pada esp32 c3 super mini sebelumnya
1. Copy project ini
2. Buka project ini di Arduino IDE
3. Pilih board "ESP32 C3 Dev Module"
4. Lakukan Upload dan tunggu sampai selesai
        
