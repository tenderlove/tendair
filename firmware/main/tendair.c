#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "sen66_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"

static const char *TAG = "tendair";

void app_main(void)
{
    printf("Hello world!\n");

    i2c_master_bus_handle_t bus_handle;
    sensirion_i2c_hal_init(&bus_handle);
    ESP_LOGI(TAG, "I2C initialized successfully");

    // Starts the sensor
    sen66_start_continuous_measurement();

    int8_t *data = (int8_t *)malloc(64);

    while (true) {
        // Get the product name
        if (!sen66_get_product_name(data, 32)) {
            printf("read: %s\n", data);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
