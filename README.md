# Server ESP32 
- C
- FreeRTOS
- [BME680 driver library](https://github.com/UncleRus/esp-idf-lib/tree/master/components/bme680)
- [esp_idf_lib_helpers](https://github.com/UncleRus/esp-idf-lib/tree/master/components/esp_idf_lib_helpers)
- [i2cdev](https://github.com/UncleRus/esp-idf-lib/tree/master/components/i2cdev)

# Client Ubuntu
- C


## Functions:
- Reading data from the BME680 sensor on ESP32 and sending it to a client running on Ubuntu.
- In case of multiple simultaneous connections, a queue is created.
- An LED indicates an incoming connection from a client.
