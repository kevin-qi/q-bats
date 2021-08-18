

int pinArray[8] = {22,23,24,25,26,27,28,29};
int count = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for (int i=0;i<8;i++){
    pinMode(pinArray[i], OUTPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i=0; i<8; i++){
    displayBinary(count);
    count++;
    delay(100);
  }
  
}

void displayBinary(byte numToShow)
{
  for (int i =0;i<8;i++)
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
