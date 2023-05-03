
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include <Ethernet.h>
#include <ThingsBoard.h>

float conPM25;
unsigned long durationPM25;
unsigned long starttime;
unsigned long sampletime_ms = 15000;
unsigned long lowpulseoccupancyPM25 = 0;

float co2Value = 0;
uint8_t CO2Pin = A3;
float coValue = 0;
uint8_t COPin = A2;
uint8_t PM25PIN = 6;

#define I2C_ADDRESS 0x3C
#define RST_PIN -1
SSD1306AsciiAvrI2c oled;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192,)

EthernetClient ethClient;


PubSubClient mqttClient(ethClient);

#define TOKEN "mj2QNlWkM2ISjGMz9NCU" 
char thingsboardServer[] = "demo.thingsboard.io";
ThingsBoard tb(ethClient);

void setup(){
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(COPin, INPUT);
  pinMode(PM25PIN,INPUT);
  pinMode(CO2Pin, INPUT);
 #if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
  #else // RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
  #endif // RST_PIN >= 0

  oled.setFont(Adafruit5x7);

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }    
  print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());

  delay(1500);
}
void loop(){
    
 if ( !tb.connected() ) {
    while (!tb.connected()) {
      Serial.print("Connecting to ThingsBoard node ...");
      // Attempt to connect (clientId, username, password)
      if ( tb.connect(thingsboardServer, TOKEN) ) {
        Serial.println( "[DONE]" );
      }
      else {
        Serial.print( "[FAILED]" );
        Serial.println( " : retrying in 5 seconds" );
        // Wait 5 seconds before retrying
        delay( 5000 );
      }
    }
  }  
  co2Value = analogRead(CO2Pin);
  displayDataOLED("CO2: ", co2Value, "");
  tb.sendTelemetryFloat("co2", co2Value);
 
  delay(5000);
  
  coValue = analogRead(COPin);
  displayDataOLED("CO: ", coValue, "");
  tb.sendTelemetryFloat("co", coValue);

  tb.loop();

  starttime = millis();
  while ((millis() - starttime) < sampletime_ms){  //Only after 30s has passed we calcualte hthe ratio
    durationPM25 = pulseIn(PM25PIN, LOW);    
    lowpulseoccupancyPM25 += durationPM25;
  }  

  tb.loop();

  conPM25 = calculateConcentration(lowpulseoccupancyPM25,15);
  lowpulseoccupancyPM25 = 0;
 
  displayDataOLED("PM2.5: ", conPM25, " ug/m3");
  tb.sendTelemetryFloat("pm25", conPM25);
    
  delay(5000);
   
  }

void displayDataOLED(String gas, float conc, String gasUnit){
  oled.clear();
  oled.set2X();
  oled.setCursor(50,0);
  oled.print(gas);
  if(gasUnit != "") {
    oled.setCursor(0, 4);
    oled.print(conc);
    oled.print(gasUnit);
  }else{
    oled.setCursor(25,4);
    oled.print(conc);
  }
  Serial.println(gas + conc + gasUnit);
}

float calculateConcentration(long lowpulseInMicroSeconds, int durationinSeconds){
  
  float ratio = (lowpulseInMicroSeconds/1000000.0)/durationinSeconds*100; //Calculate the ratio
  float concentration = 0.001915 * pow(ratio,2) + 0.09522 * ratio - 0.04884;//Calculate the concentration
  Serial.print("lowpulseoccupancy:");
  Serial.print(lowpulseInMicroSeconds);
  Serial.print("    ratio:");
  Serial.print(ratio);
  Serial.print("    Concentration:");
  Serial.println(concentration);
  return concentration;
}


  
