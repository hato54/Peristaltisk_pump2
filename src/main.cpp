// 2022-05-22 HT
// program för att stya en peristaltisk pump
// På x sekunder av y minuter
// Kommando serieport sekunder,minuter-> ENTER
// Skriv "y"-> ENTER för att spara
// Tryckknapp att välja hastighet intryckt = hög, annars låg, läses in vid uppstart

#include <Arduino.h>
#include <EEProm.h>


#define LED 13
#define PWM 9   // Pin 9 used as speed
#define SPEED_OFF 0
#define SPEED_PIN  8   // Pin 8 connected speed switch
#define OVERFLOW (unsigned long)4294967295

struct Values{
  int seconds;
  int minutes;
};

Values Myvalues;

String inbuffer = "";
String sub ="";
String temp_string;

unsigned long time_base_ms;
int time_base_sec = 0;
int time_base_min = 0;
int Motor_on_sec = 0;
int Motor_off_sec = 0;
int number = 0;
int Minutes = 0;
int speed = 127;    // Low
char character;
bool value_legal = false;

bool Motor_ON = false;
bool Run = true;





void setup() {


  Serial.begin(9600);
  pinMode(SPEED_PIN, INPUT_PULLUP);
  pinMode(PWM, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  

 /* 
  Myvalues.seconds = 55;
  Myvalues.minutes = 66;
  EEPROM.put(0, Myvalues);
  delay(100);*/
  EEPROM.get(0, Myvalues);
  Serial.print("Motor PÅ ");Serial.print(Myvalues.seconds); Serial.print(" sekunder, ");
  Serial.print("Motor AV ");Serial.print(Myvalues.minutes); Serial.println(" minuter ");

  
  time_base_ms = millis();

  Motor_ON = true;
  Serial.println("Motor PÅ");
  digitalWrite(LED, HIGH);
  
  if(digitalRead(SPEED_PIN)){
    speed = 127;  // Half power
  } else{
    speed = 255;   // Full power
  } 
  analogWrite(PWM, speed);
  
}

void loop() {

  // Check for new setup
  if(Serial.available() != 0){
    Run = false;
    //inbuffer = Serial.readString();
    character = (char)(Serial.read());
    inbuffer += character;
    Serial.print(character);

    if(inbuffer.endsWith("\n")){ 
      if(value_legal == true && 0 == inbuffer.indexOf('y') ){   // Clear to save values
        EEPROM.put(0, Myvalues);
        Motor_on_sec = 0;
        Motor_off_sec = 0;
        Motor_ON = true;
        Serial.print( Myvalues.seconds);Serial.print(", ");Serial.print( Myvalues.minutes);Serial.println(" saved!");
        value_legal = false;
        Run = true;
        digitalWrite(LED, HIGH);
        analogWrite(PWM, speed);
      }else{
        number = inbuffer.indexOf(','); 
        sub = inbuffer.substring(0,number);
        Myvalues.seconds = sub.toInt();
        sub ="";
        sub = inbuffer.substring(number+1);
        Myvalues.minutes = sub.toInt();
        if(number != -1 && inbuffer.length() > 3 &&  Myvalues.seconds > 0 &&  Myvalues.minutes > 0){
          Serial.print( Myvalues.seconds); Serial.print(" sekunder, ");
          Serial.print( Myvalues.minutes); Serial.println(" minuter?");
          Serial.println( "\'y\' to save");
          value_legal = true;
        }else{
          Serial.println("Inmatningsfel");
          value_legal = false;
          //Run = true;
        }
      }inbuffer = "";
    }
  } //End of Check for new setup

    // Time base
  if(millis() >= time_base_ms){
    if(millis() - time_base_ms >= 1000){
      time_base_ms = millis();
      time_base_sec++;        //Serial.println(time_base_sec);
      Motor_on_sec++;
      Motor_off_sec++;
      if(time_base_sec % 60 == 0){
        time_base_min++;      //Serial.println(time_base_min);
        }
      if(time_base_sec == 3600){
        time_base_sec = 0;
      }
    }
  }else{  // millis() restarted after 50 days
    if((OVERFLOW - time_base_ms + millis()) >= 1000){
      time_base_ms = millis();
      time_base_sec++; Serial.println("Overflow!");
      Motor_on_sec++;
      Motor_off_sec++;
      if(time_base_sec % 60 == 0){
        time_base_min++; 
      }
      if(time_base_min >= 1440){  // Reset after 24 hours
        time_base_min = 0;
      }
      if(time_base_sec == 3600){  // Reset after one hour
        time_base_sec = 0;
      }
    }
  }  // End of Timebase

  if(Run){  
    if(Motor_on_sec >= Myvalues.seconds && Motor_ON){
      Motor_ON = false;
      Motor_off_sec = 0;
      //Motor_off_min = 0;
      digitalWrite(LED, LOW);
      analogWrite(PWM, SPEED_OFF);
      Serial.print(Motor_on_sec); Serial.println(" sekunder -> Motor AV");
    }
    if(Motor_off_sec / 60  >= Myvalues.minutes && !Motor_ON){
      Motor_ON = true;
      Motor_on_sec = 0;
      digitalWrite(LED, HIGH);
      analogWrite(PWM, speed); 
      Serial.print(Motor_off_sec / 60);Serial.println(" minuter -> Motor På");
    }
  }
 
  
}




