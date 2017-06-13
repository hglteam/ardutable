int stepPin = 3;
int directionPin = 4;
int relayPin = 5;

int tMax = 1000; //[us] 600
int tMin = 1; //[us] 30
int rampLength = 400; //[steps] 250
double acceleration = 10; //[RPSPS]
int inputDegrees = 0;
int position = 0;
int stepTime = 0;

boolean direction = true; //(true means CW)
boolean stop = false;
boolean available = false;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

boolean connected = false;

//motor turns per table turn = 180 turns
//

void jumpStep();
void rampUp(int);
void rampDown(int);
void moveDegrees(int,boolean);
void readSerial();
void connection();
void correctPosition();

void setup() {  
  pinMode(stepPin,OUTPUT);
  pinMode(directionPin,OUTPUT);
  pinMode(relayPin,OUTPUT);
    // initialize serial:
  Serial.begin(9600);
  inputString.reserve(100);
  //variables
  acceleration = (tMax - tMin)/rampLength;
}

void loop() {
   readSerial();
   correctPosition();
   delay(1);
   //if(connected) moveDegrees(inputDegrees,direction);
}

void jumpStep(){
  delayMicroseconds(stepTime/2);
  digitalWrite(stepPin,HIGH);
  delayMicroseconds(stepTime/2);
  digitalWrite(stepPin,LOW);
  //readSerial();
}

void rampUp(){ 
   int i = 1;
   while(i<=rampLength){
    //stepTime = tMax - (int)(acceleration*i);
    stepTime = tMax - 5*i/2 + 120;
    //Serial.println(stepTime);
    jumpStep();
    i++;
   }  
}

void rampDown(){
   int i = 1;
   while(i<=rampLength){
    //stepTime = tMin + (int)(acceleration*i);
    stepTime = -5/2 + 5*i/2 + 120;
    jumpStep();
    i++;
   }
}

void moveDegrees(int degrees,boolean cw){
  int i = 0;
  int motionLength = 0;
  int currentDegrees = 0;
  digitalWrite(directionPin,cw);
  if(!stop){
  rampUp();
  while(currentDegrees<degrees){
    if(degrees == 1) motionLength = 1000 - 2*rampLength;
    else if(currentDegrees == 0) motionLength = 1000 - rampLength;
    else if(degrees - currentDegrees > 2) motionLength = 1000;
    else if(degrees - currentDegrees == 1) motionLength = 1000 - rampLength;
    while(i<motionLength){
      jumpStep();
      i++;
    }
    i=0;
    currentDegrees++;
    if (cw) position++;
    else position--;
  }
  rampDown();
  }
  available = true;
}

void serialEvent() {
  while (Serial.available()) {
    char inputChar = (char)Serial.read();
    inputString += inputChar;
    if (inputChar == '\n') {
      stringComplete = true;
    }
  }
}

//COMMANDS LIST
/*
 * M0: STOP
 * M1: STEP CCW
 * M2: STEP CW
 * M3: MOVE ANGLE
 *  
 * C0: GET POSITION
 * C1: RESET ZERO
 * C2: MOVE TO ZERO
 * C3: STATUS
 * 
 */

void readSerial(){
  int currentPosition = 0;
  if (stringComplete) {
    if(inputString == "V\n"){
      Serial.print("ArduTable 1.0.0\r");
      connected = true;
      available = true;
    }
    
    else if(inputString == "M0\n" && connected){
      //inputDegrees = 0;
      //digitalWrite(stepPin,LOW);
      stop = true;
      available = true;
    }
    else if(inputString == "M1\n" && connected){
      direction = true;
      stop = false;
      available = false;
      moveDegrees(1,direction);
    }
    else if(inputString == "M2\n" && connected){
      direction = false;
      stop = false;
      available = false;
      moveDegrees(1,direction);
    }
    else if(inputString.startsWith("M3") && connected){
      stop = false;
      available = false;
      //Serial.print(inputString.substring(2,inputString.length()-1));
      if(inputString.charAt(2)=='0') moveDegrees(inputString.substring(3,inputString.length()-1).toInt(),true);
      else if(inputString.charAt(2)=='1') moveDegrees(inputString.substring(3,inputString.length()-1).toInt(),false);
    }
    else if(inputString == "C0\n" && connected){
      String output=String(position);
      Serial.print(output + "\r");
    }
    else if(inputString == "C1\n" && connected){
      position = 0;
    }
    else if(inputString == "C2\n" && connected){
      stop = false;
      available = false;
        if(position < 0 && position > -180) {
          direction = true;
          moveDegrees(-position,direction);
        }
        else if(position < 0 && position < -180) {
          direction = false;
          moveDegrees(360 + position,direction);
        }
        else if(position > 0 && position < 180){
          direction = false;
          moveDegrees(position,direction);
        }
        else if(position > 0 && position > 180){
          direction = true;
          moveDegrees(360 - position,direction);
        }
    }
    else if(inputString == "C3\n" && connected){
      Serial.print(available);
    }
    else if(inputString == "C4\n" && connected){
      digitalWrite(relayPin,HIGH);
    }
    
    else if(inputString == "C5\n" && connected){
      digitalWrite(relayPin,LOW);
    }
    inputString = "";
    stringComplete = false;
  }
}

void correctPosition(){
  if(position >= 360){
    position -= 360;
   }
   else if(position <= -360){
    position += 360;
  }
}
