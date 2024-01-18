# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ivan/projects/zephyrOS/modules/hal/espressif/components/bootloader/subproject"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf/build/bootloader"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf/tmp"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf/src/EspIdfBootloader-stamp"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf/src"
  "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf/src/EspIdfBootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf/src/EspIdfBootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ivan/projects/zephyrOS/esp32_ble_csc/build/esp-idf/src/EspIdfBootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
