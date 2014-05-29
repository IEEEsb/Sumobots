
#define FADE_TIME 100
#define DETECTION_DISTANCE_CM 20
#define DETECTION_TIME (DETECTION_DISTANCE_CM*58)
#define KEEP_TIME 200
#define THRESHOLD_MARGIN 300

int red =9;
int green =10;
int blue =11;

int IN1=5;
int IN2=4;
int IN3=6;
int IN4=7;
int enable=8;

int sensorFront=0;
int sensorRear=1;

int trigger =12;
int button =3;

int echo =2;

int thresholdf,thresholdr;
int started=0;
int triggered =0;
unsigned long ustime,stime;
byte sensors;

void buttonPressed();
void echoed();
void trig();
void forward();
void reverse();
void search();
void fade();
void leds( byte r, byte g, byte b);

void setup(){
  Serial.begin(115200);
  pinMode(trigger,OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(enable,OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(green,OUTPUT);
  pinMode(blue,OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(echo,INPUT);  
  digitalWrite(button,HIGH);
  attachInterrupt(1,buttonPressed,FALLING);
  attachInterrupt(0,echoed,CHANGE);
  while(!started) fade();
  delay(5000);
  thresholdf=analogRead(sensorFront)-THRESHOLD_MARGIN;
  thresholdr=analogRead(sensorRear)-THRESHOLD_MARGIN;
  digitalWrite(enable,HIGH);
}


void loop(){
  trig();
  sensors=0;
  if(analogRead(sensorFront)<thresholdf) sensors |=0b10;
  if(analogRead(sensorRear)<thresholdr) sensors |=0b01;
  if(sensors){
  switch (sensors){
    case 0b01:
      reverse();
      leds(0,0,128);
    break;
    case 0b10:
      forward();
      leds(0,128,128);
    break;
    case 0b11:
      search();
      leds(128,0,128);
    break;
  }
      stime=millis();
  }
  else if((millis()-stime >KEEP_TIME)){
  
  if(ustime < DETECTION_TIME){
    forward();
     leds(0,128,0);
     stime=millis();
  } 
  else {
    search(); 
    leds(128,0,0);
  }
}
}

void buttonPressed(){
 started=1; 
}

void trig(){
  if(!triggered){
  digitalWrite(trigger,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger,LOW);
  triggered = 1;
  }
}

void echoed(){
  static long utime;  
  if(digitalRead(echo) == HIGH){
      utime =micros();
    }else{
      ustime = abs(micros()-utime);
      triggered =0;
    }
}

void fade(){
  static unsigned long ptime;
  if((millis()-ptime)>FADE_TIME){
    analogWrite(red, random(0,255));
    analogWrite(green,random(0,255));
    analogWrite(blue,random(0,255));
    ptime = millis();
  }
}

void forward(){
  digitalWrite(IN1,HIGH);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN4,LOW);
}

void reverse(){
  digitalWrite(IN2,HIGH);
  digitalWrite(IN4,HIGH);
  digitalWrite(IN1,LOW);
  digitalWrite(IN3,LOW);
}

void search(){
  analogWrite(IN1,128);
  analogWrite(IN4,128);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
}

void leds( byte r, byte g, byte b){
 analogWrite(red,r); 
 analogWrite(green,g); 
 analogWrite(blue,b); 
}
