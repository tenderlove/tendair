#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "mdns.h"
#include <hap.h>
#include <hap_apple_servs.h>
#include <hap_apple_chars.h>
#include "homekit.h"

static const char *TAG = "homekit";

/* Sensor values change every second; don't spam controllers with events */
#define HOMEKIT_UPDATE_INTERVAL_US (10ULL * 1000 * 1000)

/* HomeKit "Carbon Dioxide Detected" fires an alert in the Home app, so only
 * trip it for genuinely bad levels.  Air quality below covers "stuffy". */
#define CO2_ABNORMAL_PPM 2000

/* HomeKit AirQuality characteristic values */
enum {
    AQ_UNKNOWN = 0,
    AQ_EXCELLENT,
    AQ_GOOD,
    AQ_FAIR,
    AQ_INFERIOR,
    AQ_POOR,
};

static hap_char_t *s_air_quality;
static hap_char_t *s_pm2p5;
static hap_char_t *s_pm10p0;
static hap_char_t *s_temperature;
static hap_char_t *s_humidity;
static hap_char_t *s_co2_detected;
static hap_char_t *s_co2_level;
static bool s_started;
static int64_t s_last_update_us;

static int identify(hap_acc_t *ha)
{
    ESP_LOGI(TAG, "Accessory identified");
    return HAP_SUCCESS;
}

static void hap_event_handler(void *arg, esp_event_base_t event_base, int32_t event, void *data)
{
    switch (event) {
    case HAP_EVENT_PAIRING_STARTED:
        ESP_LOGI(TAG, "Pairing started");
        break;
    case HAP_EVENT_PAIRING_ABORTED:
        ESP_LOGI(TAG, "Pairing aborted");
        break;
    case HAP_EVENT_CTRL_PAIRED:
        ESP_LOGI(TAG, "Controller %s paired. Controller count: %d",
                 (char *)data, hap_get_paired_controller_count());
        break;
    case HAP_EVENT_CTRL_UNPAIRED:
        ESP_LOGI(TAG, "Controller %s removed. Controller count: %d",
                 (char *)data, hap_get_paired_controller_count());
        break;
    case HAP_EVENT_CTRL_CONNECTED:
        ESP_LOGI(TAG, "Controller %s connected", (char *)data);
        break;
    case HAP_EVENT_CTRL_DISCONNECTED:
        ESP_LOGI(TAG, "Controller %s disconnected", (char *)data);
        break;
    default:
        break;
    }
}

/* Map a value onto Excellent..Poor using four ascending upper bounds */
static uint8_t grade(float value, const float bounds[4])
{
    for (int i = 0; i < 4; i++) {
        if (value <= bounds[i]) {
            return AQ_EXCELLENT + i;
        }
    }
    return AQ_POOR;
}

/* Overall air quality is the worst of the individual pollutants */
static uint8_t air_quality(const homekit_reading_t *r)
{
    /* Roughly US EPA AQI breakpoints (2024) */
    static const float pm2p5_bounds[4] = { 9.0, 35.4, 55.4, 125.4 };
    static const float pm10_bounds[4] = { 54, 154, 254, 354 };
    /* Sensirion VOC / NOx index: 100 (VOC) and 1 (NOx) are "normal" */
    static const float voc_bounds[4] = { 100, 150, 250, 400 };
    static const float nox_bounds[4] = { 20, 50, 150, 300 };
    static const float co2_bounds[4] = { 800, 1000, 1400, 2000 };

    uint8_t q = grade(r->pm2p5, pm2p5_bounds);
    uint8_t g;
    if ((g = grade(r->pm10p0, pm10_bounds)) > q) q = g;
    if ((g = grade(r->voc_index, voc_bounds)) > q) q = g;
    if ((g = grade(r->nox_index, nox_bounds)) > q) q = g;
    if ((g = grade(r->co2, co2_bounds)) > q) q = g;
    return q;
}

static float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static void update_float(hap_char_t *hc, float v)
{
    hap_val_t val = { .f = v };
    hap_char_update_val(hc, &val);
}

static void update_int(hap_char_t *hc, int v)
{
    hap_val_t val = { .i = v };
    hap_char_update_val(hc, &val);
}

void homekit_update(const homekit_reading_t *r)
{
    if (!s_started) {
        return;
    }

    int64_t now = esp_timer_get_time();
    if (s_last_update_us && now - s_last_update_us < HOMEKIT_UPDATE_INTERVAL_US) {
        return;
    }
    s_last_update_us = now;

    /* Round to the characteristic step sizes so tiny jitter doesn't count as
     * a change, and clamp to the ranges the characteristics accept (out of
     * range updates are rejected outright). */
    update_int(s_air_quality, air_quality(r));
    update_float(s_pm2p5, clampf(roundf(r->pm2p5), 0, 1000));
    update_float(s_pm10p0, clampf(roundf(r->pm10p0), 0, 1000));
    update_float(s_temperature, clampf(roundf(r->temperature * 10) / 10, 0, 100));
    update_float(s_humidity, clampf(roundf(r->humidity), 0, 100));
    update_float(s_co2_level, r->co2);
    update_int(s_co2_detected, r->co2 >= CO2_ABNORMAL_PPM ? 1 : 0);
}

void homekit_init(void)
{
    uint8_t mac[6];
    char serial[13];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(serial, sizeof(serial), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    /* Append part of the MAC to the name so multiple sensors don't collide */
    hap_cfg_t hap_cfg;
    hap_get_config(&hap_cfg);
    hap_cfg.unique_param = UNIQUE_NAME;
    hap_set_config(&hap_cfg);

    if (hap_init(HAP_TRANSPORT_WIFI) != HAP_SUCCESS) {
        ESP_LOGE(TAG, "hap_init failed");
        return;
    }

    hap_acc_cfg_t cfg = {
        .name = CONFIG_HOMEKIT_NAME,
        .manufacturer = "Tendair",
        .model = "Tendair SEN66",
        .serial_num = serial,
        .fw_rev = "1.0.0",
        .hw_rev = NULL,
        .pv = "1.1.0",
        .identify_routine = identify,
        .cid = HAP_CID_SENSOR,
    };
    hap_acc_t *accessory = hap_acc_create(&cfg);
    hap_acc_add_wifi_transport_service(accessory, 0);

    hap_serv_t *service;

    service = hap_serv_air_quality_sensor_create(AQ_UNKNOWN);
    hap_serv_add_char(service, hap_char_name_create("Air Quality"));
    s_pm2p5 = hap_char_pm_2_5_density_create(0);
    s_pm10p0 = hap_char_pm_10_density_create(0);
    hap_serv_add_char(service, s_pm2p5);
    hap_serv_add_char(service, s_pm10p0);
    s_air_quality = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_AIR_QUALITY);
    hap_acc_add_serv(accessory, service);

    service = hap_serv_temperature_sensor_create(0);
    hap_serv_add_char(service, hap_char_name_create("Temperature"));
    s_temperature = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_CURRENT_TEMPERATURE);
    hap_acc_add_serv(accessory, service);

    service = hap_serv_humidity_sensor_create(0);
    hap_serv_add_char(service, hap_char_name_create("Humidity"));
    s_humidity = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_CURRENT_RELATIVE_HUMIDITY);
    hap_acc_add_serv(accessory, service);

    service = hap_serv_carbon_dioxide_sensor_create(0);
    hap_serv_add_char(service, hap_char_name_create("Carbon Dioxide"));
    s_co2_level = hap_char_carbon_dioxide_level_create(0);
    hap_serv_add_char(service, s_co2_level);
    s_co2_detected = hap_serv_get_char_by_uuid(service, HAP_CHAR_UUID_CARBON_DIOXIDE_DETECTED);
    hap_acc_add_serv(accessory, service);

    hap_add_accessory(accessory);

    hap_set_setup_code(CONFIG_HOMEKIT_SETUP_CODE);
    hap_set_setup_id(CONFIG_HOMEKIT_SETUP_ID);

    esp_event_handler_register(HAP_EVENT, ESP_EVENT_ANY_ID, &hap_event_handler, NULL);

    if (hap_start() != HAP_SUCCESS) {
        ESP_LOGE(TAG, "hap_start failed");
        return;
    }
    s_started = true;

    /* hap_start() sets the mDNS hostname to a hardcoded "MyHost", which
     * collides when there's more than one sensor on the network */
    char hostname[16];
    snprintf(hostname, sizeof(hostname), "tendair-%02x%02x%02x", mac[3], mac[4], mac[5]);
    mdns_hostname_set(hostname);
    ESP_LOGI(TAG, "mDNS hostname: %s.local", hostname);

    ESP_LOGI(TAG, "Paired with %d controllers", hap_get_paired_controller_count());
    ESP_LOGI(TAG, "Setup code: %s", CONFIG_HOMEKIT_SETUP_CODE);
    char *payload = esp_hap_get_setup_payload(CONFIG_HOMEKIT_SETUP_CODE,
                                              CONFIG_HOMEKIT_SETUP_ID, false, cfg.cid);
    if (payload) {
        ESP_LOGI(TAG, "Setup payload (turn into a QR code to scan): %s", payload);
        free(payload);
    }
}
