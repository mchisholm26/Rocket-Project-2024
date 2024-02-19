#include <SD.h>
#include <Wire.h>
#include <SparkFun_KX13X.h>
#include <Adafruit_MPL3115A2.h>

SparkFun_KX134 kxAccel;
Adafruit_MPL3115A2 mpl;

outputData accelData;

void setup()
{
    Wire.begin();

    Serial.begin(115200);
    Serial.println("welcome, i guess");

    while (!Serial)
        delay(50); // Wait for Serial Monitor to open

    if (!kxAccel.begin())
    {
        Serial.println("Could not communicate with the the KX13X! Going to just... stop.");
        while (1)
            ;
    }

    if (!mpl.begin())
    {
        Serial.println("Could not communicate with the the MPL3115A2! Going to just... stop.");
        while (1)
            ;
    }

    delay(5);

    kxAccel.enableAccel(false);

    kxAccel.setRange(SFE_KX134_RANGE16G); // 16g

    kxAccel.enableDataEngine(); // Enables the bit that indicates data is ready.
    // kxAccel.setOutputDataRate(); // Default is 50Hz
    kxAccel.enableAccel();

    mpl.setMode(MPL3115A2_BAROMETER);
    mpl.setSeaPressure(1013.26);
}

void loop()
{
    if (kxAccel.dataReady())
    {
        kxAccel.getAccelData(&myData);
        Serial.print("X: ");
        Serial.print(myData.xData, 4);
        Serial.print(" Y: ");
        Serial.print(myData.yData, 4);
        Serial.print(" Z: ");
        Serial.print(myData.zData, 4);
        Serial.println();
    }

    delay(20); // Delay for accel should be 1/ODR (Output Data Rate), default is 1/50ODR
    float pressure = baro.getPressure();
    float altitude = baro.getAltitude();
    float temperature = baro.getTemperature();

    Serial.println("-----------------");
    Serial.print("pressure = ");
    Serial.print(pressure);
    Serial.println(" hPa");
    Serial.print("altitude = ");
    Serial.print(altitude);
    Serial.println(" m");
    Serial.print("temperature = ");
    Serial.print(temperature);
    Serial.println(" C");
}
