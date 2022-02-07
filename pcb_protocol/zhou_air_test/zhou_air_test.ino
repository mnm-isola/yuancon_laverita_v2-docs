int AirPins[6] = {2,3,4,5,6,7};
int curr = 0;


void setup() {
  for (int i=0; i<6; i++){
    pinMode(AirPins[i], OUTPUT);
  }
}

void loop() {
  for (int i=0; i<6; i++){
    digitalWrite(AirPins[i], LOW);
  }

  digitalWrite(AirPins[curr], HIGH);
  curr++;
  if (curr > 6){
    curr=0;
  }
  delay(500);
}
