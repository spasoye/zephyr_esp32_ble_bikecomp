# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ivan/projects/zephyrOS/modules/hal/espressif/components/partition_table"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf/build"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/zephyr/soc/soc/xtensa/espressif_esp32/esp32/EspPartitionTable-prefix"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/zephyr/soc/soc/xtensa/espressif_esp32/esp32/EspPartitionTable-prefix/tmp"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/zephyr/soc/soc/xtensa/espressif_esp32/esp32/EspPartitionTable-prefix/src/EspPartitionTable-stamp"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/zephyr/soc/soc/xtensa/espressif_esp32/esp32/EspPartitionTable-prefix/src"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/zephyr/soc/soc/xtensa/espressif_esp32/esp32/EspPartitionTable-prefix/src/EspPartitionTable-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/zephyr/soc/soc/xtensa/espressif_esp32/esp32/EspPartitionTable-prefix/src/EspPartitionTable-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/zephyr/soc/soc/xtensa/espressif_esp32/esp32/EspPartitionTable-prefix/src/EspPartitionTable-stamp${cfgdir}") # cfgdir has leading slash
endif()
