idf_component_register(SRCS "tendair.c" "sen66_i2c.c" "sensirion_common.c"
        "sensirion_i2c_hal.c" "sensirion_i2c.c"
                    INCLUDE_DIRS "."
                    REQUIRES espressif__cjson esp_driver_i2c esp_wifi esp_netif mqtt nvs_flash)
