## DHT11/22 for Pine A64

This is simple C application that reads DHT11/22 sensor using one wire protocol.

### Compile

Compile directly on Pine A64.

```bash
apt-get install build-essential
make
sudo make install
```

### Use it

Find a PINE GPIO number: http://joey.hazlett.us/pine64/pine64_pins.html

Execute in terminal:

```bash
dht22 -p <pin-number> -d 1000
```

It will read DHT connected on <pin-number> value every 1000ms continuously.

### Read specific value

```bash
dht22 -p <pin-number> -s hum
dht22 -p <pin-number> -s temp
```

### Use with Home-Assistant

Add this to your configuration file:

```
sensor:
- platform: command_line
  name: office_room
  unit_of_measurement: "°C"
  command: "dht22 -r 10 -s temp -p 71"
- platform: command_line
  name: office_room
  unit_of_measurement: "humidity"
  command: "dht22 -r 10 -s hum -p 71"
```

### Dependencies

For controlling the GPIO's I used this RPi.GPIO-PineA64:
https://github.com/swkim01/RPi.GPIO-PineA64/blob/master/source/c_gpio.c

## Author

Kamil Trzciński, ayufan@ayufan.eu
