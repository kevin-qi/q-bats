

int pinArray[17] = {22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38};
int numLED = 17;
int count = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for (int i=0;i<numLED;i++){
    pinMode(pinArray[i], OUTPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  for (int i=0; i<numLED; i++){
    digitalWrite(pinArray[i], HIGH);
    delay(1000);
    digitalWrite(pinArray[i], LOW);
  }*/
  displayBinary(count);
  count++;
  delay(100);
  
}

void displayBinary(byte numToShow)
{
  for (int i =0;i<numLED;i++)
  {
    if (bitRead(numToShow, i)==1)
    {
      digitalWrite(pinArray[i], HIGH);
    }
    else
    {
      digitalWrite(pinArray[i], LOW);
    }
  }

}
