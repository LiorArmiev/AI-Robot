#include <BatReader.h>
#include <US.h>
#include <OttoSerialCommand.h>
#include <Otto.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "Arduino.h"

int incomingByte = 0;
void printDetail(uint8_t type, int value);
SoftwareSerial mp3(10, 11); // RX, TX

DFRobotDFPlayerMini myDFPlayer;

OttoSerialCommand SCmd;
Otto Otto;

  #define PIN_YL 2
  #define PIN_YR 3
  #define PIN_RL 4
  #define PIN_RR 5
  #define PIN_Buzzer 13
  #define ECHOPIN 9
  #define TRIGPIN 8

unsigned long previousMillis=0;

///////////////////////////////////////////////////////////////////
//-- Global Variables -------------------------------------------//
///////////////////////////////////////////////////////////////////

const char programID[]="Otto_APP_1"; //Each program will have a ID

//-- Movement parameters
int T=1000;              //Initial duration of movement
int moveId=0;            //Number of movement
int modeId=0;            //Number of mode
int moveSize=15;         //Asociated with the height of some movements

int randomDance=0;
int randomSteps=0;
int REDled = 0;
int GREENled = 0;
int BLUEled = 0;
bool obstacleDetected = false;
int ob = 1;
int data111;
///////////////////////////////////////////////////////////////////
//-- Setup ------------------------------------------------------//
///////////////////////////////////////////////////////////////////
void setup(){
  //Serial communication initialization    
    Serial.begin(9600 );        
    Serial.println("Start");
    
    mp3.begin(9600);
    mp3.listen();
    myDFPlayer.begin(mp3);
        
    unsigned long previousMillis=0;
  //Set the servo pins
  Otto.init(PIN_YL, PIN_YR, PIN_RL, PIN_RR, true, false, PIN_Buzzer,false, true);

  //Set a random seed
  randomSeed(analogRead(A6));
  //Setup callbacks for SerialCommand commands 
  SCmd.addCommand("S", receiveStop);
  SCmd.addCommand("M", receiveMovement);
  SCmd.addCommand("H", receiveGesture);
  SCmd.addCommand("C", receiveTrims);
  SCmd.addCommand("G", receiveServo);
  SCmd.addCommand("A", receiveArnold);
  SCmd.addCommand("B", receiveBanana);
  SCmd.addCommand("L", receiveLior);
  SCmd.addCommand("V", receiveVador);
  SCmd.addCommand("Q", receiveSound);
  SCmd.addDefaultHandler(receiveStop);
  //Otto wake up!
  Otto.sing(S_connection);
  Otto.home();

  delay(200);
  playSound(15,25);
  do
  {
   delay(500);
  } while((myDFPlayer.available())==0);
  
  
  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);
}
///////////////////////////////////////////////////////////////////
//-- Principal Loop ---------------------------------------------//
///////////////////////////////////////////////////////////////////
void loop() {
  //Serial.println(millis());
  obstacleDetector(); //always check obstecals
  SCmd.readSerial(); 
  
//  if (millis()-previousMillis>=80000){
  //            OttoSleeping_withInterrupts(); //ZZzzzzz...
    //          previousMillis=millis();         
      //    }
  //Serial.println(millis());
        if(obstacleDetected){
              Serial.println("obstacleDetected");

              switch (ob) {
                case 1:
                  playSound(13,25);
                  Otto.jump(5, 500);
                  Otto.walk(1,1300,-1);
                  ob = 2;
                  break;
                case 2:
                  playSound(15,25);
                  Otto.bend(1,500,1);
                  Otto.walk(1,1300,-1);                  
                  Otto.jump(5, 500);
                  ob = 3;
                  break;
                case 3:
                  playSound(16,25);
                  Otto.ascendingTurn(1,1000,30);
                  Otto.walk(1,1300,-1);
                  ob = 1;
                  break;  
                default:          
                  ob = 1;
                break;
              }
              
              delay(100);

            Otto.home();
            
        }
        //else{
              //If Otto is moving yet
       //       if (Otto.getRestState()==false){
       //         move(moveId);
       //       }
       //       else
       //       {
       //         delay(1000);
       //         Serial.print("Random Walk");
       //         randomDance=random(5,21); //5,20
       //         delay(1000);
       //         if((randomDance>14)&&(randomDance<19)){
       //             randomSteps=1;
       //             T=1600;
       //         }
       //         else{
       //             randomSteps=random(3,6); //3,5
       //             T=1000;
       //         }
       //         Serial.print(randomDance);
        //        move(randomDance);
                //for (int i=0;i<randomSteps;i++){
                //    move(randomDance);
                //}
        //      }
       // }
        delay(1000);
}  

///////////////////////////////////////////////////////////////////
//-- Functions --------------------------------------------------//
///////////////////////////////////////////////////////////////////

//-- Function to receive Stop command.
void receiveStop(){

    //sendAck();
    //Serial.print("Stop");
    Otto.sing(S_disconnection);
    Otto.home();
    //sendFinalAck();
}


//-- Function to receive TRims commands
void receiveTrims(){  

    //sendAck & stop if necessary
    sendAck();
    Otto.home(); 

    int trim_YL,trim_YR,trim_RL,trim_RR;

    //Definition of Servo Bluetooth command
    //C trim_YL trim_YR trim_RL trim_RR
    //Examples of receiveTrims Bluetooth commands
    //C 20 0 -8 3
    bool error = false;
    char *arg;
    arg=SCmd.next();
    if (arg != NULL) { trim_YL=atoi(arg); }    // Converts a char string to an integer   
    else {error=true;}

    arg = SCmd.next(); 
    if (arg != NULL) { trim_YR=atoi(arg); }    // Converts a char string to an integer  
    else {error=true;}

    arg = SCmd.next(); 
    if (arg != NULL) { trim_RL=atoi(arg); }    // Converts a char string to an integer  
    else {error=true;}

    arg = SCmd.next(); 
    if (arg != NULL) { trim_RR=atoi(arg); }    // Converts a char string to an integer  
    else {error=true;}
    
    if(error==true){

      Otto.putMouth(xMouth);
      delay(2000);
      Otto.clearMouth();

    }else{ //Save it on EEPROM
      Otto.setTrims(trim_YL, trim_YR, trim_RL, trim_RR);
      Otto.saveTrimsOnEEPROM(); //Uncomment this only for one upload when you finaly set the trims.
    } 

    sendFinalAck();

}

//-- Function to receive Servo commands
void receiveServo(){  

    sendAck(); 
    moveId = 30;

    //Definition of Servo Bluetooth command
    //G  servo_YL servo_YR servo_RL servo_RR 
    //Example of receiveServo Bluetooth commands
    //G 90 85 96 78 
    bool error = false;
    char *arg;
    int servo_YL,servo_YR,servo_RL,servo_RR;

    arg=SCmd.next();
    if (arg != NULL) { servo_YL=atoi(arg); }    // Converts a char string to an integer   
    else {error=true;}

    arg = SCmd.next(); 
    if (arg != NULL) { servo_YR=atoi(arg); }    // Converts a char string to an integer  
    else {error=true;}

    arg = SCmd.next(); 
    if (arg != NULL) { servo_RL=atoi(arg); }    // Converts a char string to an integer  
    else {error=true;}

    arg = SCmd.next(); 
    if (arg != NULL) { servo_RR=atoi(arg); }    // Converts a char string to an integer  
    else {error=true;}
    
    if(error==true){
    }else{ //Update Servo:
      int servoPos[4]={servo_YL, servo_YR, servo_RL, servo_RR}; 
      Otto._moveServos(200, servoPos);   //Move 200ms      
    }
    sendFinalAck();
}


//-- Function to receive movement commands
void receiveMovement(){
    Serial.print("Move");
    sendAck();

    if (Otto.getRestState()==true){
        Otto.setRestState(false);
    }

    //Definition of Movement Bluetooth commands
    //M  MoveID  T   MoveSize  
    char *arg; 
    arg = SCmd.next(); 
    if (arg != NULL) {moveId=atoi(arg);}
    else{
      moveId=0; //stop
    }
        
    arg = SCmd.next(); 
    if (arg != NULL) {T=atoi(arg);}
    else{
      T=1000;
    }

    arg = SCmd.next(); 
    if (arg != NULL) {moveSize=atoi(arg);}
    else{
      moveSize =15;
    }
    move(moveId);
}


//-- Function to execute the right movement according the movement command received.
void move(int moveId){

  bool manualMode = false;

  switch (moveId) {
    case 0:
      Otto.home();
      break;
    case 1: //M 1 1000 
      Otto.walk(1,T,1);
      break;
    case 2: //M 2 1000 
      Otto.walk(1,T,-1);
      break;
    case 3: //M 3 1000 
      Otto.turn(1,T,1);
      break;
    case 4: //M 4 1000 
      Otto.turn(1,T,-1);
      break;
    case 5: //M 5 1000 30 
      Otto.updown(1,T,moveSize);
      break;
    case 6: //M 6 1000 30
      Otto.moonwalker(1,T,moveSize,1);
      break;
    case 7: //M 7 1000 30
      Otto.moonwalker(1,T,moveSize,-1);
      break;
    case 8: //M 8 1000 30
      Otto.swing(1,T,moveSize);
      break;
    case 9: //M 9 1000 30 
      Otto.crusaito(1,T,moveSize,1);
      break;
    case 10: //M 10 1000 30 
      Otto.crusaito(1,T,moveSize,-1);
      break;
    case 11: //M 11 1000 
      Otto.jump(1,T);
      break;
    case 12: //M 12 1000 30 
      Otto.flapping(1,T,moveSize,1);
      break;
    case 13: //M 13 1000 30
      Otto.flapping(1,T,moveSize,-1);
      break;
    case 14: //M 14 1000 20
      Otto.tiptoeSwing(1,T,moveSize);
      break;
    case 15: //M 15 500 
      Otto.bend(1,T,1);
      break;
    case 16: //M 16 500 
      Otto.bend(1,T,-1);
      break;
    case 17: //M 17 500 
      Otto.shakeLeg(1,T,1);
      break;
    case 18: //M 18 500 
      Otto.shakeLeg(1,T,-1);
      break;
    case 19: //M 19 500 20
      Otto.jitter(1,T,moveSize);
      break;
    case 20: //M 20 500 15
      Otto.ascendingTurn(1,T,moveSize);
      break;
    default:
        manualMode = true;
      break;
  }

  if(!manualMode){
    sendFinalAck();
  }
       
}


//-- Function to receive gesture commands
void receiveGesture(){
    Serial.print("Gesture");
    //sendAck & stop if necessary
    sendAck();
    Otto.home(); 

    //Definition of Gesture Bluetooth commands
    //H  GestureID  
    int gesture = 0;
    char *arg; 
    arg = SCmd.next(); 
    if (arg != NULL) {gesture=atoi(arg);}
    else 
    {
      //Otto.putMouth(xMouth);
      delay(2000);
      //Otto.clearMouth();
    }

    switch (gesture) {
      case 1: //H 1 
        Otto.playGesture(OttoHappy);
        playSound(2,25);
        break;
      case 2: //H 2 
        playSound(6,25);
        Otto.playGesture(OttoSuperHappy);
        break;
      case 3: //H 3 
        playSound(4,25);
        Otto.playGesture(OttoSad);
        break;
      case 4: //H 4 
        Otto.playGesture(OttoSleeping);
        break;
      case 5: //H 5  
        Otto.playGesture(OttoFart);
        break;
      case 6: //H 6 
        Otto.playGesture(OttoConfused);
        break;
      case 7: //H 7 
        Otto.playGesture(OttoLove);
        break;
      case 8: //H 8 
        Otto.playGesture(OttoAngry);
        break;
      case 9: //H 9  
        Otto.playGesture(OttoFretful);
        break;
      case 10: //H 10
        Otto.playGesture(OttoMagic);
        break;  
      case 11: //H 11
        Otto.playGesture(OttoWave);
        break;   
      case 12: //H 12
        Otto.playGesture(OttoVictory);
        break; 
      case 13: //H 13
        Otto.playGesture(OttoFail);
        break;         
      default:
        break;
    }

    sendFinalAck();
}


void receiveArnold(){
  Serial.print("A");
  playSound(8,28);   
  Otto.shakeLeg(1,T,-1);
  Otto.shakeLeg(1,T,1);
  Otto.home(); 
  
}

void receiveLior(){
  Serial.print("L");
  playSound(6,20); 
  Otto.playGesture(OttoSuperHappy);
  Otto.home(); 
}

void receiveBanana(){
  Serial.print("B");
  playSound(5,30);
  Otto.ascendingTurn(1,T,moveSize);
  Otto.shakeLeg(1,T,-1);
  Otto.home();
}
void receiveVador(){
  Serial.print("V");
  playSound(18,25);
  Otto.swing(1,T,moveSize);
  Otto.crusaito(1,T,moveSize,1);
  Otto.ascendingTurn(1,T,moveSize);
  Otto.crusaito(1,T,moveSize,-1);
  Otto.ascendingTurn(1,T,moveSize);
  Otto.swing(1,T,moveSize);
  Otto.swing(1,T,moveSize);
  Otto.crusaito(1,T,moveSize,1);
  Otto.home(); 
  
}

//-- Function to send ultrasonic sensor measure (distance in "cm")
void requestDistance(){

    Otto.home();  //stop if necessary  
    int distance = Otto.getDistance();
    Serial.print(F("&&"));
    Serial.print(F("D "));
    Serial.print(distance);
    Serial.println(F("%%"));
    //Serial.flush();
}

//-- Function to send Ack comand (A)
void sendAck(){

  delay(30);
  Serial.println(F("A"));
  //Serial.flush();
}


//-- Function to send final Ack comand (F)
void sendFinalAck(){

  delay(30);
  Serial.println(F("F"));
  //Serial.flush();
}

//-- Functions with animatics
//--------------------------------------------------------


void OttoSleeping_withInterrupts(){
  int bedPos_0[4]={100, 80, 60, 120};
  Otto._moveServos(700, bedPos_0);
  for(int i=0; i<4;i++){
    Otto.bendTones (100, 200, 1.04, 10, 10);
    Otto.bendTones (200, 300, 1.04, 10, 10);
    Otto.bendTones (300, 500, 1.04, 10, 10);
    delay(500);
    Otto.bendTones (400, 250, 1.04, 10, 1);
    Otto.bendTones (250, 100, 1.04, 10, 1);
    delay(500);
    }
  Otto.sing(S_cuddly);
  Otto.home();
}

void obstacleDetector(){
// Start Ranging
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
// Compute distance
  float distance = pulseIn(ECHOPIN, HIGH);
  distance= distance/58;
  //Serial.print(distance);
  //Serial.println("cm");
 if(distance<15){
          obstacleDetected = true;
          //Serial.println ("Obstacle Detected! Avoid collision");
        }else{
          obstacleDetected = false;
          //Serial.println ("No Obstacle detected! - Keep on walking");
        }
  
}

void receiveSound()
{
      int sound;
      char *arg;
      arg=SCmd.next();
      sound=atoi(arg);
      Serial.println(sound);
      playSound(sound,30);
}

void playSound(int song, int vol){

  mp3.listen();
  //Serial.println(myDFPlayer.available());
  myDFPlayer.volume(vol);
  myDFPlayer.play(song);
  //do
  //{
  // delay(500);
  //} while((myDFPlayer.available())==0);
  //
}
