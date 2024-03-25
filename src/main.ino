#include <SD.h>
#include <Wire.h>
#include <Timer.h>
#include <RH_RF95.h>
#include <SparkFun_KX13X.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPL3115A2.h>

#define RADIO_FREQ 915.0

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

RH_RF95 rf95(RFM95_CS, RFM95_INT);
SparkFun_KX134 kxAccel;
Adafruit_MPL3115A2 mpl;
Adafruit_BNO055 bno = Adafruit_BNO055(55);

// struct for mpl data
struct mplData
{
    float pressure;
    float altitude;
    float temperature;
};

outputData accelData;
mplData baroData;

void writeHeaders()
{
    if (SD.exists("datalog.csv"))
    {
        SD.remove("datalog.csv"); // remove the file if it exists

File file;

Timer logTimer;
Timer accelTimer;
Timer baroTimer;
Timer bnoTimer;

    }
    file = SD.open("datalog.csv", FILE_WRITE);

    file.println("DeltaTime,AccelAge,AccelX,AccelY,AccelZ,BaroAge,Pressure,Altitude,Temperature");
    file.close();
    // DeltaTime is the time between the last data point and the current one.
    // AccelX, AccelY, AccelZ are the accelerometer data.
    // Pressure, Altitude, Temperature are the barometer data.
    // AccelAge and BaroAge are the time since the last data point for each sensor.
}

void writeDataPoint()
{
    file = SD.open("datalog.csv", FILE_WRITE);
    file.print(logTimer.read());
    file.print(",");
    file.print(accelTimer.read());
    file.print(",");
    file.print(accelData.xData);
    file.print(",");
    file.print(accelData.yData);
    file.print(",");
    file.print(accelData.zData);
    file.print(",");
    file.print(baroTimer.read());
    file.print(",");
    file.print(baroData.altitude);
    file.print(",");
    file.print(baroData.pressure);
    file.print(",");
    file.print(baroData.temperature);
    file.print(",");
    file.print(bnoTimer.read());
    // TODO: add BNO055 data
    file.println();
    file.close();
    logTimer.start(); // reset the timer!
}

void setup()
{
    Wire.begin();

    Serial.begin(115200);
    Serial.println("welcome, i guess");

    delay(50);  // Wait for Serial Monitor to recognize us if we're connected directly...
    // make sure we have access to all of our sensors / sd card before starting...

    if (!kxAccel.begin())
    {
    if (!rf95.init()) {
        Serial.println("Could not communicate with the radio! Going to just... stop.");
        while (1)
            ;
    }

    // do radio setup first, so if we have any issues later we can scream those problems

    if (!rf95.setFrequency(RADIO_FREQ)) {
        Serial.println("Could not communicate with the radio! Going to just... stop.");
        while (1)
            ;
    }

    rf95.setTxPower(23, false);

    // TODO: scream errors over radio

        Serial.println("Could not communicate with the KX13X! Going to just... stop.");
        while (1)
            ;
    }

    if (!mpl.begin())
    {
        Serial.println("Could not communicate with the MPL3115A2! Going to just... stop.");
        while (1)
            ;
    }

    if (!bno.begin())
    {
    	Serial.println("Could not communicate with the BNO055! Going to just... stop");
    	while (1)
    		;
    }

    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println("Card failed, or not present");
        while (1)
            ;
    }

    baroData.pressure = 0;
    baroData.altitude = 0;
    baroData.temperature = 0;

    writeHeaders();

    delay(5);

    kxAccel.enableAccel(false);

    kxAccel.setRange(SFE_KX134_RANGE16G); // 16g

    kxAccel.enableDataEngine(); // Enables the bit that indicates data is ready.
    // kxAccel.setOutputDataRate(); // Default is 50Hz
    kxAccel.enableAccel();

    mpl.setMode(MPL3115A2_BAROMETER);
    mpl.setSeaPressure(1013.26); // calibration! unit should be hPa.

    bno.setExtCrystalUse(true);

    logTimer.start();
    accelTimer.start();
    baroTimer.start();
    bnoTimer.start();
    mpl.startOneShot(); // Start the sensor in one-shot mode.
}

void loop()
{
    if (kxAccel.dataReady())
    {
        kxAccel.getAccelData(&accelData);
        accelTimer.start(); // reset the timer
    }

    if (mpl.conversionComplete()) {
        baroTimer.start(); // reset the timer
        baroData.pressure = mpl.getLastConversionResults(MPL3115A2_PRESSURE);
        baroData.altitude = mpl.getLastConversionResults(MPL3115A2_ALTITUDE);
        baroData.temperature = mpl.getLastConversionResults(MPL3115A2_TEMPERATURE);
        mpl.startOneShot(); // get latest data
    }
    writeDataPoint();
}
