// Transmitter code, used for testing AND real time data collection
// Reads sensor data
// Prints temperature, pressure and humidity
// Sends data in "temperature;pressure;humidity"
// Prints confirmation message from receiver
// Prints RSSI

#include <SPI.h>
#include <RH_RF95.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// RFM pins
#define RFM95_RST 5
#define RFM95_INT 3
#define RFM95_CS 4

// BME pins
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define LED 13

#define RF95_FREQ 433.0
#define SEALEVELPRESSURE_HPA (1013.25)

// instance of sensor driver
Adafruit_BME280 bme(BME_CS); // hardware SPI

// instance of radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(9600);
  delay(100);

  Serial.println("Starting BME280 test");
  if (!bme.begin()) {
      Serial.println("BME280 init failed");
      while (1);
  }
  Serial.println("BME280 init OK");

  Serial.println("Starting LoRa TX test");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); 
  Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
  
  digitalWrite(BME_CS, HIGH);
  digitalWrite(RFM95_CS, HIGH);
}

void loop()
{
  digitalWrite(BME_CS, LOW);
  Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");
    char temp[8];
    dtostrf(bme.readTemperature(), 1, 2, temp);
    
    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    char humidity[8];
    dtostrf(bme.readHumidity(), 1, 2, humidity);

    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");
    char pressure[8];
    dtostrf(bme.readPressure()/100.0, 1, 2, pressure);

    Serial.println();
  digitalWrite(BME_CS, HIGH);
  digitalWrite(RFM95_CS, LOW);
  
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server

  char fullMessage[30];
  sprintf(fullMessage, "%s;%s;%s\n", temp, humidity, pressure);
  
  Serial.print("Sending message: "); 
  Serial.println(fullMessage);
  
  Serial.println("Sending..."); 
  delay(10);

  rf95.send((uint8_t *)fullMessage, 30);

  Serial.println("Waiting for packet to complete..."); 
  delay(10);
  rf95.waitPacketSent();
  
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply..."); 
  delay(10);

  if (rf95.waitAvailableTimeout(10000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("Receive failed");
    }
    
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
  }
  
  digitalWrite(RFM95_CS, HIGH);
  delay(10000);
}
