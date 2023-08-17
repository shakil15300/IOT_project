#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);
#include <LoRa.h>
 
#define ss 5
#define rst 14
#define dio0 2
 
void setup() 
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");
 
  LoRa.setPins(ss, rst, dio0);    //setup LoRa transceiver module
 
  while (!LoRa.begin(433E6))     //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");
  lcd.begin();  
  lcd.backlight();
  lcd.setCursor(0,1);
  delay(500); 
}
 
void loop() 
{
  int packetSize = LoRa.parsePacket();    // try to parse packet
  if (packetSize) 
  {
    
    Serial.print("Received packet '");
 
    while (LoRa.available())              // read packet
    {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData);
      lcd.clear();// clear previous values from screen
      lcd.print(LoRaData);
      delay(500);
    }
    Serial.print("' with RSSI ");         // print RSSI of packet
    Serial.println(LoRa.packetRssi());
  }
}

