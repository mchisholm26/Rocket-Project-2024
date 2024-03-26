#include <RH_RF95.h>

#define RADIO_FREQ 915.0

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup()
{
    Serial.begin(9600);

    while (!Serial)
        delay(100);

    Serial.println("ok: serial connected");

    if (!rf95.init())
    {
        Serial.println("error: initializing radio");
        while (1)
            ;
    }

    if (!rf95.setFrequency(RADIO_FREQ))
    {
        Serial.println("error: tuning radio to frequency");
        while (1)
            ;
    }

    rf95.setTxPower(23, false);

    Serial.println("ok: radio connected");
}

void loop()
{
    // read incoming messages from rocket
    char buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
        Serial.print("message: ");
        Serial.println((char *)buf);
        Serial.print("rssi: ");
        Serial.println(rf95.lastRssi(), DEC);
    }
}
