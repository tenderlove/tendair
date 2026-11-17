# Flashing firmware

First [install esp-idf](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html).

HomeKit support comes from [esp-homekit-sdk](https://github.com/espressif/esp-homekit-sdk),
which is a git submodule, so make sure it's checked out:

```
git submodule update --init
```

Then do:

```
idf.py set-target esp32-c3
idf.py menuconfig
```

Select `Tendair Configuration` and change the Wifi SSID and Wifi password.
Set the MQTT Prefix, I use "home/main/office" for my office that is on the
main floor of my house.

Also set the MQTT uri, which should be the URI of the MQTT server you've set up.

Optionally change the HomeKit name, setup code, and setup ID.  The default
setup code is `111-22-333`.

Then:

```
idf.py build flash
```

## Writing to InfluxDB

Use `tools/mqtt-to-influx.rb` to subscribe to the messages and write them to
InfluxDB

## HomeKit

The sensor shows up in the Home app as one accessory with four sensors:

* Air Quality (with PM2.5 and PM10 densities)
* Temperature
* Humidity
* Carbon Dioxide (with the CO2 level)

The Air Quality rating (Excellent .. Poor) is the worst of PM2.5, PM10, VOC
index, NOx index, and CO2.  The thresholds are in `main/homekit.c`.  "Carbon
Dioxide Detected" trips at 2000 ppm, since the Home app treats it as an alert.

To pair, open the Home app, tap "+" > "Add Accessory" > "More options...",
pick Tendair, and enter the setup code.  The setup payload is also logged at
boot if you want to turn it into a QR code.

If the accessory doesn't show up (or shows up briefly and disappears), check
for WiFi 7 MLO on your router.  Some routers don't forward multicast to MLO
clients, so the phone never sees the sensor's Bonjour announcements.  Turn MLO
off while pairing.  With a home hub (Apple TV / HomePod) that isn't on MLO,
it's fine to turn it back on afterwards.

Pairing info is stored in NVS.  To unpair / start over, remove the accessory
in the Home app, or run `idf.py erase-flash` and reflash.

### Upgrading from a pre-HomeKit build

The HomeKit build needs the full 4MB of flash and a custom partition table,
which existing `sdkconfig` files don't have (settings in `sdkconfig.defaults`
only apply to options that aren't already set).  Either delete `sdkconfig` and
redo `idf.py menuconfig`, or in menuconfig set:

* Serial flasher config > Flash size > 4 MB
* Partition Table > Custom partition table CSV (`partitions.csv`)
* Component config > ESP System Settings > Main task stack size > 8192
* Component config > LWIP > Max number of open sockets > 16

The partition table changed, so do a full `idf.py flash` (not just `app-flash`).
