#include <ArduinoJson.h>

int buttonPin = 46;
int ledPin = 48;

int buttonState = 0;
int timeDelay = 50;

void setup() {
  pinMode(ledPin,OUTPUT);
  pinMode(buttonPin,INPUT);
  Serial.begin(9600);
  Serial.setTimeout(10);
}

void loop() {
  buttonState = digitalRead(buttonPin);
  
  StaticJsonBuffer<500> jsonBuffer;

  JsonObject& SN1 = jsonBuffer.createObject();

  SN1["sectorID"] = "1";
  SN1["deviceID"] = "1";
  SN1["lat"] = "43.010429";
  SN1["lng"] = "-83.715630";
  SN1["time"] = "0";

  SN1.printTo(Serial);
  Serial.write("\n");
  //delay(timeDelay);

  if(buttonState == HIGH)
  {
    JsonObject& SN2 = jsonBuffer.createObject();

    SN2["sectorID"] = "1";
    SN2["deviceID"] = "2";
    SN2["lat"] = "43.010604";
    SN2["lng"] = "-83.716253";
    SN2["time"] = "999";
  
    SN2.printTo(Serial);
    Serial.write("\n");
    //delay(timeDelay);

    JsonObject& SN4 = jsonBuffer.createObject();

    SN4["sectorID"] = "1";
    SN4["deviceID"] = "4";
    SN4["lat"] = "43.010201";
    SN4["lng"] = "-83.716633";
    SN4["time"] = "0";
  
    SN4.printTo(Serial);
    Serial.write("\n");
    //delay(timeDelay);

    JsonObject& CH1 = jsonBuffer.createObject();

    CH1["sectorID"] = "1";
    CH1["deviceID"] = "5";
    CH1["lat"] = "0";
    CH1["lng"] = "0";
    CH1["time"] = "0";
  
    CH1.printTo(Serial);
    Serial.write("\n");
  }
  else
  {
    JsonObject& SN2 = jsonBuffer.createObject();

    SN2["sectorID"] = "1";
    SN2["deviceID"] = "2";
    SN2["lat"] = "43.009571";
    SN2["lng"] = "-83.715563";
    SN2["time"] = "0";
  
    SN2.printTo(Serial);
    Serial.write("\n");
    //delay(timeDelay);
  }

  JsonObject& SN3 = jsonBuffer.createObject();

  SN3["sectorID"] = "1";
  SN3["deviceID"] = "3";
  SN3["lat"] = "43.009568";
  SN3["lng"] = "-83.718836";
  SN3["time"] = "0";

  SN3.printTo(Serial);
  Serial.write("\n");

  JsonObject& CH2 = jsonBuffer.createObject();

  CH2["sectorID"] = "2";
  CH2["deviceID"] = "1";
  CH2["lat"] = "0";
  CH2["lng"] = "0";
  CH2["time"] = "0";

  CH2.printTo(Serial);
  Serial.write("\n");
  //delay(timeDelay);
}
