cmake_minimum_required(VERSION 3.13)

# == Pico SDK ==
include(pico_sdk_import.cmake)

project(cnn_mnist C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico_w CACHE STRING "Pico board")

pico_sdk_init()

# --- Configuração do TFLM (Cópia do professor) ---
set(BUILD_TESTING OFF CACHE BOOL "Disable building tests" FORCE)
set(PICO_TFLMICRO_BUILD_TESTS OFF CACHE BOOL "Disable pico-tflmicro tests" FORCE)
set(PICO_TFLMICRO_BUILD_EXAMPLES OFF CACHE BOOL "Disable pico-tflmicro examples" FORCE)
set(PICO_TFLMICRO_BUILD_BENCHMARKS OFF CACHE BOOL "Disable pico-tflmicro benchmarks" FORCE)

add_subdirectory(pico-tflmicro pico-tflmicro-build EXCLUDE_FROM_ALL)

set(TFLM_TARGET "")
foreach(candidate IN ITEMS pico_tflmicro pico-tflmicro tflmicro pico_tflmicro_lib)
    if(TARGET ${candidate})
        set(TFLM_TARGET ${candidate})
        break()
    endif()
endforeach()

if(TFLM_TARGET STREQUAL "")
    message(FATAL_ERROR "Could not find a pico-tflmicro library target.")
endif()

# --- Sua Aplicação ---

add_executable(cnn_mnist
    classificacao-imagens.c
    tflm_wrapper.cpp  # O wrapper do professor entra aqui
)

pico_set_program_name(cnn_mnist "cnn_mnist")
pico_set_program_version(cnn_mnist "0.1")

# Habilitar USB e Desabilitar UART (para focar na USB)
pico_enable_stdio_uart(cnn_mnist 1)
pico_enable_stdio_usb(cnn_mnist 1)

target_include_directories(cnn_mnist PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(cnn_mnist PRIVATE
    pico_stdlib
    ${TFLM_TARGET}

)

pico_add_extra_outputs(cnn_mnist)