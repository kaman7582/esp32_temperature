# esp32_temperature
temperature sensor

step:

1.enable gpio to collect data 

hw： DS18B20

GPIO ->18

red->vcc

yellow->data

black->groud

create a task to read the tempeature sensor

2.connect the device to wifi
https://github.com/mongoose-os-libs/mqtt
deploy mqtt

3.set mqtt to upload data to server
https://mongoose-os.com/docs/quickstart/cloud/google.md
connect to the google server

4.server analyse the data
