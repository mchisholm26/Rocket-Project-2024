//Interface description: https://content.u-blox.com/sites/default/files/u-blox-M9-SPG-4.04_InterfaceDescription_UBX-21022436.pdf#%5B%7B%22num%22%3A2477%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C59.527%2C599.697%2Cnull%5D

// Might take some configuration ahead of time
// It looked like it was *once* reading partial NMEA sentences but not anymore.

#include <SPI.h>

const int gps_select = 4;
const int gps_reset = 5;
const int gps_freq = 1000000;
const int gps_spi_mode = SPI_MODE0;
const int gps_bit_order = MSBFIRST;

void setup_gps() {
  pinMode(gps_select, OUTPUT);
  pinMode(gps_reset, OUTPUT);
  digitalWrite(gps_select, HIGH);
  digitalWrite(gps_select, HIGH);

  // digitalWrite(gps_reset, LOW);
  // delay(125);
  // digitalWrite(gps_reset, HIGH);

  delay(700);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();

  while (!Serial)
  {
    delay(10);
  }
  Serial.println("Starting...");
  Serial8.begin(38400);

  //setup_gps();
  //Serial.println("GPS setup.");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial8.available()) {
    Serial.printf("%c", Serial8.read());
  }
}
