#pragma once

#include <stdint.h>

/* Readings in real units (already unscaled from the SEN66 integer format) */
typedef struct {
    float pm2p5;        /* µg/m³ */
    float pm10p0;       /* µg/m³ */
    float humidity;     /* %RH */
    float temperature;  /* °C */
    float voc_index;    /* Sensirion VOC index, 1..500 */
    float nox_index;    /* Sensirion NOx index, 1..500 */
    uint16_t co2;       /* ppm */
} homekit_reading_t;

/* Create the HomeKit accessory and start the HAP server. Call after wifi /
 * esp_netif / the default event loop are up. */
void homekit_init(void);

/* Push a new set of readings to HomeKit. Cheap to call often; values are
 * rate limited internally. */
void homekit_update(const homekit_reading_t *reading);
