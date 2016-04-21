int buttonState = 0;
String input;

int buttonPin = 46;
int ledPin = 48;

void setup() {
  pinMode(ledPin,OUTPUT);
  pinMode(buttonPin,INPUT);
  Serial.begin(9600);
  Serial.setTimeout(10);
}

void loop() {
  buttonState = digitalRead(buttonPin);

  if(buttonState == HIGH)
  {
    Serial.write("HIGH**");
  }
  else
  {
    Serial.write("LOW**");
  }

  input = Serial.readString();

  if (input.equals("99"))
  {
    digitalWrite(ledPin, LOW);
  }
  else if (input.equals("11"))
  {
    digitalWrite(ledPin, HIGH);
  }
  
}
