import sys
import time
import os

# Menambahkan tool-esptoolpy ke sys.path
sys.path.insert(0, r"D:\.platformio\packages\tool-esptoolpy")

# Monkey-patch serialwin32 agar tidak crash jika SetCommState menghasilkan WinError 31
try:
    import serial
    from serial.serialwin32 import Serial as Win32Serial, win32
    import ctypes

    orig_reconfigure = Win32Serial._reconfigure_port
    def patched_reconfigure(self):
        try:
            orig_reconfigure(self)
        except Exception as e:
            # Abaikan error 31 (ERROR_GEN_FAILURE) pada native USB ESP32-S3
            pass
    Win32Serial._reconfigure_port = patched_reconfigure

    # Juga abaikan error pada _set_dtr / _set_rts jika gagal
    orig_set_dtr = Win32Serial._set_dtr
    def patched_set_dtr(self, value):
        try:
            orig_set_dtr(self, value)
        except Exception:
            pass
    Win32Serial._set_dtr = patched_set_dtr

    orig_set_rts = Win32Serial._set_rts
    def patched_set_rts(self, value):
        try:
            orig_set_rts(self, value)
        except Exception:
            pass
    Win32Serial._set_rts = patched_set_rts

except Exception as ex:
    print("[WARN] Gagal monkey-patch serial:", ex)

import esptool

if __name__ == "__main__":
    port = "COM19"
    if len(sys.argv) > 1:
        port = sys.argv[1]

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

    print("=== SMARTBUS PASSENGER MONITOR FLASHER ===")
    print(f"Target Port : {port}")
    print("Mencoba menghubungkan ke ESP32-S3...")
    
    sys.argv = cmd
    try:
        esptool._main()
        print("\n[SUCCESS] Firmware SmartBus Passenger berhasil di-flash ke ESP32-S3!")
    except Exception as e:
        print("\n[ERROR] Gagal flashing:", e)
        sys.exit(1)
