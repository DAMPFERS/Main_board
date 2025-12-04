cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME test_MX)
include("cmake/gcc-arm-none-eabi.cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME})
enable_language(C ASM)
add_executable(${CMAKE_PROJECT_NAME})

add_subdirectory(cmake/stm32cubemx)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Inc
    Drivers/STM32F0xx_HAL_Driver/Inc
    Drivers/CMSIS/Device/ST/STM32F0xx/Include
    Drivers/CMSIS/Include
)

# Добавьте только пользовательские исходники
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/main.c
    Core/Src/stm32f0xx_it.c
    Core/Src/stm32f0xx_hal_msp.c
    Core/Src/sysmem.c
    Core/Src/syscalls.c
    startup_stm32f030x6.s
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim_ex.c
)

target_link_libraries(${CMAKE_PROJECT_NAME} stm32cubemx)