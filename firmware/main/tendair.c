#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static const char *TAG = "tendair";

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          CONFIG_I2C_MASTER_FREQUENCY
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       1000

#define SEN66_SENSOR_ADDR         0x6B

static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SEN66_SENSOR_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

/**
 * @brief Read a sequence of bytes from a SEN66 sensor registers
 */
static esp_err_t sen66_register_read(i2c_master_dev_handle_t dev_handle, uint8_t *reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev_handle, reg_addr, 2, data, len, I2C_MASTER_TIMEOUT_MS);
}

void app_main(void)
{
    printf("Hello world!\n");

    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
    i2c_master_init(&bus_handle, &dev_handle);
    ESP_LOGI(TAG, "I2C initialized successfully");

    // Starts the sensor
    uint8_t start_samples[2] = { 0x00, 0x21 };
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, start_samples, sizeof(start_samples), I2C_MASTER_TIMEOUT_MS));

    uint8_t *data = (uint8_t *)malloc(64);

    while (true) {
        // Get the product name
        uint8_t product_name[2] = { 0xD0, 0x14 };
        ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, product_name, sizeof(product_name), I2C_MASTER_TIMEOUT_MS));
        esp_rom_delay_us(20 * 1000);
        ESP_ERROR_CHECK(i2c_master_receive(dev_handle, data, 48, -1));
        printf("read: %s\n", data);

        // Sleep for 1s
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
