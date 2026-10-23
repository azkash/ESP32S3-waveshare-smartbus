import sys
import time
import os
import serial.tools.list_ports

sys.path.insert(0, r"D:\.platformio\packages\tool-esptoolpy")

try:
    from serial.serialwin32 import Serial as Win32Serial
    orig_reconfigure = Win32Serial._reconfigure_port
    def patched_reconfigure(self):
        try:
            orig_reconfigure(self)
        except Exception:
            pass
    Win32Serial._reconfigure_port = patched_reconfigure
except Exception:
    pass

import esptool

def attempt_flash(port):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    local_bin_dir = os.path.join(script_dir, ".pio", "build", "WAVESHARE-ESP-LCD")
    bin_dir = local_bin_dir if os.path.exists(local_bin_dir) else r"D:\projects\ESP32S3\__ui_project_name__\.pio\build\WAVESHARE-ESP-LCD"
    bootloader = os.path.join(bin_dir, "bootloader.bin")
    partitions = os.path.join(bin_dir, "partitions.bin")
    boot_app0 = r"D:\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin"
    firmware = os.path.join(bin_dir, "firmware.bin")

    cmd = [
        "esptool.py",
        "--chip", "esp32s3",
        "--port", port,
        "--baud", "460800",
        "--before", "default_reset",
        "--after", "hard_reset",
        "write_flash",
        "-z",
        "--flash_mode", "dio",
        "--flash_freq", "80m",
        "--flash_size", "detect",
        "0x0000", bootloader,
        "0x8000", partitions,
        "0xe000", boot_app0,
        "0x10000", firmware
    ]

    sys.argv = cmd
    try:
        esptool._main()
        return True
    except SystemExit as se:
        if se.code == 0:
            return True
        return False
    except BaseException:
        return False

if __name__ == "__main__":
    print("=== AUTO-FLASHER SMARTBUS PASSENGER (ESP32-S3) ===")
    print("Menunggu ESP32 masuk ke Bootloader Mode...")
    print("Langkah: TAHAN tombol BOOT -> TEKAN tombol RESET sekali -> LEPAS tombol BOOT.")
    
    timeout_sec = 90
    start_time = time.time()
    
    while time.time() - start_time < timeout_sec:
        candidate_ports = [
            p.device for p in serial.tools.list_ports.comports()
            if "USB" in (p.description or "") or "303A" in (p.hwid or "") or "ESP" in (p.description or "")
        ]
        
        for port in candidate_ports:
            print(f"\n[TERDETEKSI] Mencoba flashing ke {port}...")
            if attempt_flash(port):
                print(f"\n>>> FLASH BERHASIL DI {port}! Firmware terbaru terpasang! <<<")
                sys.exit(0)
                
        remaining = int(timeout_sec - (time.time() - start_time))
        print(f"\r[Menunggu Bootloader ({remaining}s)] Tekan & tahan BOOT -> tekan RESET -> lepas BOOT...", end="", flush=True)
        time.sleep(0.5)
        
    print("\n\n[TIMEOUT] Waktu tunggu habis. Silakan ulangi.")
    sys.exit(1)
