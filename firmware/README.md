# Flashing firmware

First [install esp-idf](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html).

Then do:

```
idf.py set-target esp32-c3
idf.py menuconfig
```

Select `Tendair Configuration` and change the Wifi SSID and Wifi password.
Set the MQTT Prefix, I use "/home/main/office" for my office that is on the
main floor of my house.

Also set the MQTT uri, which should be the URI of the MQTT server you've set up.

Then:

```
idk.py build flash
```

## Writing to InfluxDB

Use `tools/mqtt-to-influx.rb` to subscribe to the messages and write them to
InfluxDB
