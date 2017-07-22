#
# ArduinoToolchain.cmake
#

set(CMAKE_SYSTEM_NAME Arduino)

set(ARDUINO_PROGRAM_DIR "C:\Program Files (x86)\Arduino")
set(ARDUINO_USERDATA_DIR $ENV{LOCALAPPDATA}\Arduino15)


set(ARDUINO_DUE_HARDWARE_VERSION 1.6.11)
set(ARDUINO_DUE_HARDWARE_DIR ${ARDUINO_USERDATA_DIR}\packages\hardware\sam\${ARDUINO_DUE_HARDWARE_VERSION})

set(ARDUINO_DUE_LIBRARIES_DIR ${ARDUINO_DUE_HARDWARE_DIR}\libraries)



set(ARDUINO_WORKING_DIR D:\Development\Arduino)
set(ARDUINO_WORKING_LIBS_DIR ${ARDUINO_WORKING_DIR}\libraries)



set(ARDUINO_LIBRARIES_DIR)
list(APPEND ARDUINO_LIBRARIES_DIR ${ARDUINO_DUE_LIBRARIES_DIR}
                                  ${ARDUINO_WORKING_LIBS_DIR})


set(ARDUINO_ARM_GCC_VER 4.8.3-2014q1)
set(ARDUINO_ARM_GCC_DIR ${ARDUINO_USERDATA_DIR}\packages\arduino\tools\arm-none-eabi-gcc\${ARDUINO_ARM_GCC_VER})

set(CMAKE_C_COMPILER   ${ARDUINO_ARM_GCC_DIR}\bin\arm-none-eabi-gcc.exe)
set(CMAKE_CXX_COMPILER ${ARDUINO_ARM_GCC_DIR}\bin\arm-none-eabi-g++.exe)


if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/Platform/Arduino.cmake)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR})
endif()
