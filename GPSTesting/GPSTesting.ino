#include <SPI.h>

const int builtin_led = 13;

const int gps_select = 4;
const int gps_reset = 5;
const int gps_freq = 100000;
const int gps_spi_mode = SPI_MODE0;
const int gps_bit_order = MSBFIRST;

void setup_gps() {
  pinMode(gps_select, OUTPUT);
  pinMode(gps_reset, OUTPUT);
  digitalWrite(gps_select, HIGH);

  digitalWrite(gps_reset, LOW);
  delay(10);
  digitalWrite(gps_reset, HIGH);
}

uint8_t read_byte_from_gps() {
  Serial.println("Here");
  SPI.beginTransaction(SPISettings(gps_freq, gps_bit_order, gps_spi_mode));
  Serial.println("Here");
  digitalWrite(gps_select, LOW);
  Serial.println("Here");
  uint8_t b = SPI.transfer(255);
  Serial.println("Here");
  digitalWrite(gps_select, HIGH);
  SPI.endTransaction();
  return b;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  setup_gps();
  
  pinMode(builtin_led, OUTPUT);

  while (!Serial) {
    digitalWrite(builtin_led, HIGH);
  }
  digitalWrite(builtin_led, LOW);

  Serial.println("Hello");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  Serial.println("Reading...");
  Serial.println(read_byte_from_gps());
  Serial.flush();
  while (!Serial) {
    digitalWrite(builtin_led, HIGH);
  }
  digitalWrite(builtin_led, LOW);
}
