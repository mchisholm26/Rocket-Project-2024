import time
import board
import adafruit_mpl3115a2

i2c = board.I2C()  # uses board.SCL and board.SDA
# i2c = board.STEMMA_I2C()  # For using the built-in STEMMA QT connector on a microcontroller
sensor = adafruit_mpl3115a2.MPL3115A2(i2c)

# You can configure the pressure at sealevel to get better altitude estimates.
# This value has to be looked up from your local weather forecast or meteorological
# reports.  It will change day by day and even hour by hour with weather
# changes.  Remember altitude estimation from barometric pressure is not exact!
# Set this to a value in hectopascals:
sensor.sealevel_pressure = 1022.5

# Main loop to read the sensor values and print them every second.
while True:
    pressure = sensor.pressure
    print("Pressure: {0:0.3f} hectopascals".format(pressure))
    altitude = sensor.altitude
    print("Altitude: {0:0.3f} meters".format(altitude))
    temperature = sensor.temperature
    print("Temperature: {0:0.3f} Celsius".format(temperature))
    time.sleep(1.0)
