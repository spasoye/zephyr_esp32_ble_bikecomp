# Bike speedometer ESP32 BLE CSC


## Flashing

```
west flash
```
### TODO
Incorporate this to gdb:
```
/usr/bin/python /home/ivan/projects/zephyrOS/modules/hal/espressif/components/esptool_py/esptool/esptool.py --chip auto --baud 921600 --before default_reset --after hard_reset write_flash -u --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 /home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf/build/bootloader/bootloader.bin 0x8000 /home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf/build/partitions_singleapp.bin 0x10000 /home/ivan/projects/zephyrOS/esp32_ble_csc/build/zephyr/zephyr.bin
```
Can gdb load .bin ?
Can gdb load multiple .bin's ?


## Debugging

```
openocd -f interface/jlink.cfg -f board/esp-wroom-32.cfg
```

```
~/bin/zephyr-sdk-0.16.4/xtensa-espressif_esp32_zephyr-elf/bin/xtensa-espressif_esp32_zephyr-elf-gdb -ex 'target extended-remote localhost:3333' build/zephyr/zephyr.elf
```

