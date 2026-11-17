# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

# Only pull in the parts of esp-homekit-sdk we use (git submodule)
set(HOMEKIT_PATH ${CMAKE_CURRENT_LIST_DIR}/esp-homekit-sdk/components/homekit)
set(EXTRA_COMPONENT_DIRS
    ${HOMEKIT_PATH}/esp_hap_core
    ${HOMEKIT_PATH}/esp_hap_apple_profiles
    ${HOMEKIT_PATH}/esp_hap_platform
    ${HOMEKIT_PATH}/hkdf-sha
    ${HOMEKIT_PATH}/mu_srp)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tendair)

# esp-homekit-sdk isn't C23 clean (IDF 6's default), so build it as gnu17...
foreach(comp esp_hap_core esp_hap_apple_profiles esp_hap_platform hkdf-sha mu_srp)
    idf_component_get_property(lib ${comp} COMPONENT_LIB)
    target_compile_options(${lib} PRIVATE -std=gnu17)
endforeach()

# ...and uses wifi interface aliases that IDF 6 removed
idf_component_get_property(lib esp_hap_core COMPONENT_LIB)
target_compile_definitions(${lib} PRIVATE ESP_IF_WIFI_STA=WIFI_IF_STA ESP_IF_WIFI_AP=WIFI_IF_AP)
