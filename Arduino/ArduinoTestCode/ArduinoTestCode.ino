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
  
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& SN1 = jsonBuffer.createObject();

  SN1["id"] = "1";
  SN1["lat"] = "-25.363";
  SN1["lon"] = "31.044";
  SN1["time"] = "0";

  SN1.printTo(Serial);
  Serial.write("\n");
  delay(timeDelay);

  if(buttonState == HIGH)
  {
    JsonObject& SN2 = jsonBuffer.createObject();

    SN2["id"] = "2";
    SN2["lat"] = "-25.363";
    SN2["lon"] = "1.044";
    SN2["time"] = "999";
  
    SN2.printTo(Serial);
    Serial.write("\n");
    delay(timeDelay);

    JsonObject& SN4 = jsonBuffer.createObject();

    SN4["id"] = "4";
    SN4["lat"] = "-25.363";
    SN4["lon"] = "41.044";
    SN4["time"] = "999";
  
    SN4.printTo(Serial);
    Serial.write("\n");
    delay(timeDelay);
  }
  else
  {
    JsonObject& SN2 = jsonBuffer.createObject();

    SN2["id"] = "2";
    SN2["lat"] = "-25.363";
    SN2["lon"] = "21.044";
    SN2["time"] = "0";
  
    SN2.printTo(Serial);
    Serial.write("\n");
    delay(timeDelay);
  }

  JsonObject& SN3 = jsonBuffer.createObject();

  SN3["id"] = "3";
  SN3["lat"] = "-25.363";
  SN3["lon"] = "11.044";
  SN3["time"] = "0";

  SN3.printTo(Serial);
  Serial.write("\n");
  delay(timeDelay);
}
