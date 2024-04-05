#include <SD.h>
#include <Wire.h>
#include <Timer.h>
#include <RH_RF95.h>
#include <SparkFun_KX13X.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPL3115A2.h>

#define RADIO_FREQ 915.0

#define RFM95_CS 1
#define RFM95_RST 9
#define RFM95_INT 2

#define KX134_CS 0

#define led_red 28     // there are red and green indicator leds on digital pins 28 and 29
#define led_green 29 

RH_RF95 rf95(RFM95_CS, RFM95_INT);
SparkFun_KX134_SPI kxAccel;
Adafruit_MPL3115A2 mpl;
Adafruit_BNO055 bno = Adafruit_BNO055(55);

long elapsedTime; // running count of milliseconds for log entries


// struct for mpl data
struct mplData
{
    float pressure;
    float altitude;
    float temperature;
};

//struct for bno data
struct bnoData
{
    imu::Vector<3> linear_acceleration; //m/s^2
    imu::Quaternion orientation_quat; //quaternion
    imu::Vector<3> angular_velocity; //rps
    uint8_t sys_health;
    uint8_t gyro_health;
    uint8_t accel_health;
};

outputData accelData;
mplData baroData;
bnoData imuData;

File file;

Timer logTimer;
Timer accelTimer;
Timer baroTimer;
Timer bnoTimer;

void writeHeaders()
{
    if (SD.exists("datalog.csv"))
    {
        SD.remove("datalog.csv"); // remove the file if it exists
    }
    file = SD.open("datalog.csv", FILE_WRITE);
    file.println("TotalTime,DeltaTime,AccelAge,AccelX,AccelY,AccelZ,BaroAge,Altitude,Pressure,Temperature,BnoAge,BnoAccelX,BnoAccelY,BnoAccelZ,BnoOrientationW,BnoOrientationX,BnoOrientationY,BnoOrientationZ,BnoAngularX,BnoAngularY,BnoAngularZ,BnoSysHealth,BnoGyroHealth,BnoAccelHealth");
    file.close();
    // DeltaTime is the time between the last data point and the current one.
    // AccelX, AccelY, AccelZ are the accelerometer data.
    // Pressure, Altitude, Temperature are the barometer data.
    // AccelAge and BaroAge are the time since the last data point for each sensor.
    // BnoAge is the time between the last data point and the current 
    // BnoAccelX, Y, Z are linear accelerations in m/s recorded by Bno
    // BnoOrientationW, X, Y, Z is absolute orientation expressed as quaternion
    // BnoAngular X, Y, Z is angular velocity expressed as an euler angle in radians per second
    // BnoSysHealth is calibration of total system (0 is bad, 3 is good)
    // BnoGyroHealth is calibration of gyro (0 is bad, 3 is good)
    // BnoAccelHealth is calibration of accelerometer (0 is bad, 3 is good)
}

void writeDataPoint()
{
    file = SD.open("datalog.csv", FILE_WRITE);
    // write the data to the buf, then send it over radio & write to SD

    elapsedTime += logTimer.read();
    
    char buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    snprintf(buf, len, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
             elapsedTime, // TotalTime
             logTimer.read(), // DeltaTime
             accelTimer.read(), // AccelAge
             accelData.xData, // AccelX
             accelData.yData, // AccelY
             accelData.zData, // AccelZ
             baroTimer.read(), // BaroAge
             baroData.altitude, // Altitude
             baroData.pressure, // Pressure
             baroData.temperature, // Temperature
             bnoTimer.read(), // BnoAge
             imuData.linear_acceleration.x(), // BnoAccelX
             imuData.linear_acceleration.y(), // BnoAccelY
             imuData.linear_acceleration.z(), // BnoAccelZ
             imuData.orientation_quat.w(), // BnoOrientationW
             imuData.orientation_quat.x(), // BnoOrientationX
             imuData.orientation_quat.y(), // BnoOrientationY
             imuData.orientation_quat.z(), // BnoOrientationZ
             imuData.angular_velocity.x(), // BnoAngularX
             imuData.angular_velocity.y(), // BnoAngularY
             imuData.angular_velocity.z(), // BnoAngularZ
             imuData.sys_health, // BnoSysHealth
             imuData.gyro_health, // BnoGyroHealth
             imuData.accel_health); // BnoAccelHealth
    
    file.println(buf);
    // prepend "d: " to the buffer to indicate that it's data
    snprintf(buf, len, "d: %s", buf);
    rf95.send((uint8_t*)buf, len);
    logTimer.start(); // reset the timer!

}

void setup()
{
    Wire.begin();

    Serial.begin(115200);
    Serial.println("welcome, i guess");

    pinMode(led_red, OUTPUT);
    pinMode(led_green, OUTPUT);
    digitalWrite(led_red, 0);   // turn on both leds to show the board is starting up
    digitalWrite(led_green, 0);

    delay(50);  // Wait for Serial Monitor to recognize us if we're connected directly...
    // make sure we have access to all of our sensors / sd card before starting...

    pinMode(KX134_CS, OUTPUT);    //this gets the kx134 cs pin ready, not sure why it's needed, maybe because it's an active low input?
    digitalWrite(KX134_CS, 1);
    pinMode(RFM95_RST, OUTPUT);   //set the radio reset pin high so the radio is ready to go
    digitalWrite(RFM95_RST, 1);
    
  //reset the radio
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  
  // do radio setup first, so if we have any issues later we can scream those problems
  if (!rf95.init()) {
      Serial.println("Could not communicate with the radio! Going to just... stop.");
      digitalWrite(led_green, 1);   // turn off green led to show that there is a problem
      while (1)
          ;
  }

  //now try to talk to the radio
  if (!rf95.setFrequency(RADIO_FREQ)) {
      Serial.println("Could not communicate with the radio! Going to just... stop.");
      digitalWrite(led_green, 1);   // turn off green led to show that there is a problem
      while (1)
          ;
  }

  rf95.setTxPower(23, false);

  //send a test message over the radio
  rf95.send("Hello, rocket here", 23);
  //delay(10);  //give the receiver a bit to catch up - I'm not sure this is necessary I'm having trouble with the receiver buffer
  
  if (!kxAccel.begin(KX134_CS))
    {
        Serial.println("Could not communicate with the KX134! Going to just... stop.");
        rf95.send("error: initializing KX134", 23);
        digitalWrite(led_green, 1);   // turn off green led to show that there is a problem
        while (1)
            ;
    }

    if (!mpl.begin())
    {
        Serial.println("Could not communicate with the MPL3115A2! Going to just... stop.");
        rf95.send("error: initializing MPL3115A2", 23);
        digitalWrite(led_green, 1);   // turn off green led to show that there is a problem
        while (1)
            ;
    }

    if (!bno.begin())
    {
    	Serial.println("Could not communicate with the BNO055! Going to just... stop");
      rf95.send("error: initializing BNO055", 26);
      digitalWrite(led_green, 1);   // turn off green led to show that there is a problem
    	while (1)
    		;
    }

    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println("Card failed, or not present");
        rf95.send("error: initializing SD card", 23);
        digitalWrite(led_green, 1);   // turn off green led to show that there is a problem
        while (1)
            ;
    }

    // everything works, send it over serial and radio
    Serial.println("Everything initialized successfully!");
    rf95.send("checks complete, ready to go!", 28);
    digitalWrite(led_red, 1);   // turn off red led to show that everything is working

    
    baroData.pressure = 0;
    baroData.altitude = 0;
    baroData.temperature = 0;

    writeHeaders();

    delay(5);

    kxAccel.enableAccel(false);

    kxAccel.setRange(SFE_KX134_RANGE64G); // 64g crank it up all the way, expecting ~40g on the flight

    kxAccel.enableDataEngine(); // Enables the bit that indicates data is ready.
    // kxAccel.setOutputDataRate(); // Default is 50Hz
    kxAccel.enableAccel();

    mpl.setMode(MPL3115A2_BAROMETER);
    mpl.setSeaPressure(1013.26); // calibration! unit should be hPa.

    bno.setExtCrystalUse(true);

    elapsedTime = 0;
    logTimer.start();
    accelTimer.start();
    baroTimer.start();
    bnoTimer.start();
    mpl.startOneShot(); // Start the sensor in one-shot mode.

    rf95.send("ok: all sensors connected", 26);
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

    //BNO055 IMU logic
    if (1) {
        bnoTimer.start();
        imuData.linear_acceleration = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
        imuData.orientation_quat = bno.getQuat();
        imuData.angular_velocity = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
        uint8_t mag = 0;
        bno.getCalibration(&imuData.sys_health, &imuData.gyro_health, &imuData.accel_health, &mag);
        
    }
    
    writeDataPoint();
}
