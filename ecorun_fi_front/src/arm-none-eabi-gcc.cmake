cmake_minimum_required(VERSION 2.6)

include(CMakeForceCompiler)
find_program(CROSS_GCC_FULL_PATH "arm-none-eabi-gcc")
get_filename_component(CROSS_COMPILE_TOOLS ${CROSS_GCC_FULL_PATH} PATH)

if (NOT EXISTS ${CROSS_COMPILE_TOOLS})
    message(FATAL_ERROR "Can't find cross compilation tool chain")
endif()

set(CPREF ${CROSS_COMPILE_TOOLS}/arm-none-eabi)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_BUILD_TYPE None)

set(CMAKE_C_COMPILER ${CPREF}-gcc CACHE STRING "arm-none-eabi-gcc" FORCE)
set(CMAKE_CXX_COMPILER ${CPREF}-g++ CACHE STRING "arm-none-eabi-g++" FORCE)
#set(CMAKE_ASM_COMPILER ${CPREF}-gcc\ -x\ -assembler-with-cpp CACHE STRING "arm-none-eabi-as" FORCE)
set(CMAKE_ASM_COMPILER ${CPREF}-gcc CACHE STRING "arm-none-eabi-as" FORCE)
set(CMAKE_AR arm-none-eabi-ar CACHE STRING "arm-none-eabi-ar" FORCE)
set(CMAKE_LD arm-none-eabi-ld CACHE STRING "arm-none-eabi-ld" FORCE)
set(OBJCOPY arm-none-eabi-objcopy CACHE STRING "arm-none-eabi-objcopy" FORCE)
set(OBJDUMP arm-none-eabi-objdump CACHE STRING "arm-none-eabi-objdump" FORCE)

cmake_force_c_compiler(arm-none-eabi-gcc GNU)
cmake_force_cxx_compiler(arm-none-eabi-g++ GNU)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

enable_language(ASM)
