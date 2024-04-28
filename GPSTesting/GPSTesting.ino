//Interface description: https://content.u-blox.com/sites/default/files/u-blox-M9-SPG-4.04_InterfaceDescription_UBX-21022436.pdf#%5B%7B%22num%22%3A2477%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C59.527%2C599.697%2Cnull%5D

// Might take some configuration ahead of time
// It looked like it was *once* reading partial NMEA sentences but not anymore.

#include <SPI.h>

const int gps_select = 4;
const int gps_reset = 5;
const int gps_freq = 5500000;
const int gps_spi_mode = SPI_MODE0;
const int gps_bit_order = MSBFIRST;

char poll_position_message[] = "PUBX,00";
char poll_satellites_message[] = "PUBX,03";
char output_rate_message[] = "PUBX,40,RMC,1,1,1,1,2,0";
const char* nmea_stop_code = "\r\n";

void setup_gps() {
  pinMode(gps_select, OUTPUT);
  pinMode(gps_reset, OUTPUT);
  digitalWrite(gps_select, HIGH);

  digitalWrite(gps_reset, LOW);
  delay(10);
  digitalWrite(gps_reset, HIGH);
}

uint8_t read_byte_from_gps() {
  SPI.beginTransaction(SPISettings(gps_freq, gps_bit_order, gps_spi_mode));
  digitalWrite(gps_select, LOW);
  uint8_t b = SPI.transfer(255);
  digitalWrite(gps_select, HIGH);
  SPI.endTransaction();
  return b;
}

void send_message_to_gps(char* message) {
  int len = strlen(message);

  Serial.println("Sending message:");

  uint8_t checksum = 0;

  SPI.beginTransaction(SPISettings(gps_freq, gps_bit_order, gps_spi_mode));
  digitalWrite(gps_select, LOW);

  SPI.transfer((uint8_t) "$");
  Serial.print("$");

  for (int i = 0; i<len; i++) {
    SPI.transfer(message[i]);
    Serial.printf("%c", message[i]);
    checksum ^= message[i];
  }

  SPI.transfer((uint8_t) "*");
  Serial.print("*");

  char cs[6];
  snprintf(cs, 5, "%02x\r\n", checksum);
 
  int l = strlen(cs);

  for (int i = 0; i < l; i++) {
    SPI.transfer(cs[i]);
    Serial.printf("%c", cs[i]);
  }

  digitalWrite(gps_select, HIGH);
  SPI.endTransaction();
}

void receive_message_from_gps(char* buffer, int len) {
  memset(buffer, 0, len);

  int place = 0;

  Serial.println("Receiving message:");
  
  SPI.beginTransaction(SPISettings(gps_freq, gps_bit_order, gps_spi_mode));
  digitalWrite(gps_select, LOW);

  //Read bytes as long as they are available and a message end has not been received. (0x0A)
  while (place < len && buffer[place-1] != 0x0A) {
    uint8_t b = SPI.transfer(0xFF);
    if (b == 0xFF) { //Try again if we overran the buffer.
      delay(10);
      Serial.println("Overran");
      continue;
    }
    buffer[place] = b;
    Serial.printf("%02x ", b);
    place += 1;
  }

  digitalWrite(gps_select, HIGH);
  SPI.endTransaction();
}

void ubx_checksum(uint8_t buffer[], int len, uint8_t* ck_a, uint8_t* ck_b) {
  *ck_a = 0;
  *ck_b = 0;
  for (int i = 0; i < len; i++) {
    *ck_a = *ck_a + buffer[i];
    *ck_b = *ck_b + *ck_a;
  }
}

void send_ubx_message(uint8_t message[], int len) {
  uint8_t ck_a;
  uint8_t ck_b;
  ubx_checksum(message, len, &ck_a, &ck_b);

  SPI.beginTransaction(SPISettings(gps_freq, gps_bit_order, gps_spi_mode));
  digitalWrite(gps_select, LOW);

  SPI.transfer(0xb5);
  SPI.transfer(0x62);

  for (int i = 0; i<len; i++) {
    SPI.transfer(message[i]);
  }

  SPI.transfer(ck_a);
  SPI.transfer(ck_b);

  digitalWrite(gps_select, HIGH);
  SPI.endTransaction();
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

  setup_gps();
}

void loop() {
  // put your main code here, to run repeatedly:

  SPI.beginTransaction(SPISettings(gps_freq, gps_bit_order, gps_spi_mode));
  digitalWrite(gps_select, LOW);

  delay(10);

  uint8_t b = SPI.transfer(0xFF);
  Serial.printf("%c", b);

  digitalWrite(gps_select, HIGH);
  SPI.endTransaction();
}
