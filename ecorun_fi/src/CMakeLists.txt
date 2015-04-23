cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/)
include(arm-none-eabi-gcc)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_BUILD_TYPE None)

set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_C_OUTPUT_EXTENSION .o)

set(CMAKE_C_FLAGS "")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb-interwork")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsigned-char")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -c")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -MMD")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -MP")

set(CMAKE_EXE_LINKER_FLAGS "-T ../lpc1347.ld -nostartfiles -nodefaultlibs -nostdlib -Xlinker --gc-sections -Wl,-Map,ecorun_fi.map")

#set(CMAKE_ASM_FLAGS "-mcpu=cortex-m3 -mthumb -O0 -mthumb-interwork" CACHE INTERNAL "asm compiler flags")

set (SystemSrcs
	system/cmsis/startup.c
	system/cmsis/system_LPC13Uxx.c
	system/cortexm/_initialize_hardware.c
    system/cortexm/_reset_hardware.c
	system/cortexm/exception_handlers.c
	system/newlib/_exit.c
	system/newlib/_sbrk.c
	system/newlib/string.c
	system/peripheral/adc.c
    system/peripheral/adc_handler.c
    system/peripheral/eeprom.c
	system/peripheral/timer.c
	system/peripheral/timer_handler.c
	system/peripheral/usart.c
	system/peripheral/usart_handler.c
    system/systimer.c
)

set (Srcs
	integer.c
	usart_receive_data_handler.c
    carsystem/accessible_data.c
    carsystem/car_info.c
    carsystem/fi_settings.c
    carsystem/injection.c
    core/adler32.c
    core/base64.c
    core/command.c
    core/json.c
    core/mempool.c
    util/eeprom_util.c
    util/usart_util.c
)

project(ecorun_fi)
add_executable(ecorun_fi ${SystemSrcs} ${Srcs} main.c)
add_custom_command(TARGET ecorun_fi COMMAND arm-none-eabi-objcopy -O ihex ecorun_fi ecorun_fi.hex)
add_custom_command(TARGET ecorun_fi COMMAND arm-none-eabi-size ecorun_fi)
add_custom_command(TARGET ecorun_fi COMMAND arm-none-eabi-size ecorun_fi.hex)

