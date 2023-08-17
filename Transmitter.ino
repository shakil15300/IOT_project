#include <LoRa.h>
#include <SPI.h>
#define ss 5
#define rst 14
#define dio0 2
#define LED_BUILTIN 2
#define SENSOR1 27
#define SENSOR2 26
#define TdsSensorPin 13
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point

int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25; 
int counter = 0;
float ph;
long currentMillis1 = 0;
long previousMillis1 = 0;
long currentMillis2= 0;
long previousMillis2= 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;

volatile byte pulseCount1;
volatile byte pulseCount2;

byte pulse1Sec = 0;
byte pulse2Sec = 0;
unsigned int flowMilliLitres1;
unsigned long totalMilliLitres1;
float flowRate1;
unsigned int flowMilliLitres2;
unsigned long totalMilliLitres2;
float flowRate2;
int c;

int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void IRAM_ATTR pulseCounter1()
{
  pulseCount1++;
}
void IRAM_ATTR pulseCounter2()
{
  pulseCount2++;
}
void setup() {
  Serial.begin(115200);
  delay(1000);
   while (!Serial);
  Serial.println("LoRa Sender");
 
  LoRa.setPins(ss, rst, dio0);    //setup LoRa transceiver module
  
  while (!LoRa.begin(433E6))     //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR1, INPUT_PULLUP);
  pinMode(SENSOR2, INPUT_PULLUP);
  pulseCount1 = 0;
  pulseCount2 = 0;
  flowRate1 = 0.0;
  flowMilliLitres1 = 0;
  totalMilliLitres1 = 0;
  previousMillis1 = 0;
  
  flowRate2 = 0.0;
  flowMilliLitres2 = 0;
  totalMilliLitres2 = 0;
  previousMillis2 = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR1), pulseCounter1, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR2), pulseCounter2, FALLING);
  pinMode(TdsSensorPin,INPUT);

}

void loop() {
    currentMillis1 = millis();
  currentMillis2 = millis();
  if((currentMillis1-previousMillis1)>interval)
  {
    pulse1Sec = pulseCount1;
    pulseCount1 = 0;

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate1 = ((1000.0/(millis()-previousMillis1))*pulse1Sec)/calibrationFactor;
    previousMillis1 = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres1 = (flowRate1 / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    //totalMilliLitres1 += flowMilliLitres1;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate-1: ");
    Serial.print(int(flowRate1));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity-1: ");
    Serial.print(flowMilliLitres1);
    Serial.print("mL \n\n");
    //Serial.print(flowMilliLitres1 / 1000);
    //Serial.println("L");
   
  }
   if((currentMillis2-previousMillis2)>interval)
  {
    pulse2Sec = pulseCount2;
    pulseCount2 = 0;

    flowRate2 = ((1000.0/(millis()-previousMillis2))*pulse2Sec)/calibrationFactor;
    previousMillis2 = millis();

    flowMilliLitres2 = (flowRate2 / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    //totalMilliLitres2 += flowMilliLitres2;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate-2: ");
    Serial.print(int(flowRate2));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity:-2 ");
    Serial.print(flowMilliLitres2);
    Serial.print("mL ");
    //Serial.print(flowMilliLitres2 / 1000);
    //Serial.println("L");
    c=flowMilliLitres2-flowMilliLitres1;
    if((c<3)&&(c>-3))
    Serial.print("no leakage");
    Serial.print("\n");
  }
  

   float Value= analogRead(35);
    float voltage1=Value*(3.3/4095.0);
    ph=(3.3*voltage1);
    Serial.println("The PH of the terminal is given by: ");
    Serial.println(ph);
    static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4096.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //temperature compensation
      float compensationVoltage=averageVoltage/compensationCoefficient;
      
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
      
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      Serial.print("TDS Value:");
      Serial.print(tdsValue,0);
      Serial.println("ppm");
    }
  }
    delay(500);
Serial.print("Sending packet: ");
  Serial.println(counter);

  LoRa.beginPacket(); 
  LoRa.print("PH Value is ");
  LoRa.print(ph);
  LoRa.print(" ");
  LoRa.print("flow 1");
  LoRa.print(flowMilliLitres1);
  LoRa.print("mL ");
  LoRa.print("flow 2");
  LoRa.print(flowMilliLitres2);
  LoRa.print("mL ");
  LoRa.print("TDS Value:");
  LoRa.print(tdsValue,0);
  LoRa.println("ppm");
  LoRa.endPacket();
 
  counter++;

 delay(5000);

}
