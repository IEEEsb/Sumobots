#define FADE_TIME 50 //Actualizacion del LED
#define DETECTION_DISTANCE_CM 50
#define DETECTION_TIME (DETECTION_DISTANCE_CM * 58) //A 340m/s, el sonido recorre 1cm en 58us
#define KEEP_TIME 800 //Tiempo minimo de accion (excepto busqueda y ataque)
#define ATTACK_TIME 800 //Tiempo minimo de ataque
#define THRESHOLD_MARGIN 300 //Histéresis sensores IR

/*Definición de pines*/

//Pines del LED
int red = 9;
int green = 10;
int blue = 11;

/*Pines motores (ver datasheet L293D)
 En realidad IN3 está conectada a 7 e IN4 a 6
 Cambiado para compensar que el motor derecho esta montado al revés */
int IN1=5;
int IN2=4;
int IN3=6;
int IN4=7;
int enable=8;

//Sensores IR
int sensorFront=1;
int sensorRear=0;

//Sensor ultrasónico (ver datasheet HC-SR04)
int trigger =12;
int echo =2;
//Pulsador
int button =3;

/*Declaración de variables globales*/
int thresholdf,thresholdr; //Umbrales sensores IR
int started=0; //Flag inicio de programa
int triggered =0; //Flag medida ultrasónico
unsigned long ustime,stime; //Tiempos ultrasónico
byte sensors; //Estado sensores IR
int searchdir=0; //Dirección de giro
/*Prototipos de funciones (no necesario en Arduino pero recomendable)*/
void buttonPressed();
void echoed();
void trig();
void forward();
void reverse();
void turn();
void fade();
void countdown();
void leds( byte r, byte g, byte b);

void setup(){
  Serial.begin(115200);
  /*Declaración de modos de pines*/
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
  digitalWrite(button,HIGH); //Necesario en versiones antiguas de Arduino para activar PullUp
  /*Interrupciones externas*/
  attachInterrupt(1,buttonPressed,FALLING);
  attachInterrupt(0,echoed,CHANGE);
  /*Espera a inicio (pulsador pulsado)*/
  while(!started) fade();
  delay(100); //Antirrebotes pulsador
  while(!digitalRead(3)) fade(); //Espera hasta que se suelte el pulsador
  countdown();//Cuenta atrás 5 segundos
  /*Autocalibración sensores IR (debe estar quieto sobre suelo negro)*/
  thresholdf=analogRead(sensorFront)-THRESHOLD_MARGIN;
  thresholdr=analogRead(sensorRear)-THRESHOLD_MARGIN;
  /*Activamos motores*/
  digitalWrite(enable,HIGH);
}


void loop(){
  trig(); //Inicia lectura ultrasonidos
  sensors=0;
  /*Lectura sensores IR*/
  if(analogRead(sensorFront)<thresholdf) sensors |=0b10;
  if(analogRead(sensorRear)<thresholdr) sensors |=0b01;
  /*Tarea 1: evitar salirse del tatami. NO puede ser bloqueada pero bloquea al resto de tareas. Acción inmediata*/
  if(sensors){ //Esta línea es igual que if (sensors != 0b00). != operador de no igualdad.
    switch (sensors){
    case 0b01: //Sensor trasero en blanco
      forward(); 
      leds(0,0,128);
      break;
    case 0b10:
      reverse(); //Sensor delantero en blanco
      leds(0,128,128);
      break;
    case 0b11: //Ambos sensores en blanco
      turn();
      leds(128,0,128);
      break;
    }
    stime=millis()+KEEP_TIME; //Tiempo actual + tiempo que bloquea la tarea (tiempo mínimo que se está haciendo)
  }
  /*Tareas que pueden ser bloqueadas*/
  else if((millis()>stime)){  //No se ejecutan hasta que el tiempo actual sea el calculado tras una tarea bloqueante
    /*Tarea 2: ataque. Se ejecuta cuando se detecta algo a distancia menor que DETECTION_DISTANCE_CM.
     Puede ser bloqueada por Tarea 1 y puede bloquearse a sí misma y a tarea 3*/
    if(ustime < DETECTION_TIME){ //Tiempo medido en ultrasonidos menor que el umbral
      forward(); //Ataque
      leds(0,128,0);
      stime=millis()+ATTACK_TIME; //Tiempo actual + tiempo de bloqueo de la tarea (tiempo mínimo que se va a estar haciendo)
      searchdir = analogRead(sensorFront)&0x01; //Dirección de giro aleatoria usando ruido del sensor de IR delanter
    } 
    /*Tarea 3: búsqueda. Consiste en girar lentamente lanzando medidas de ultrasonidos hasta detectar un obstáculo.
     Puede ser bloqueada por Tarea 1 y Tarea 2 pero no bloquea a ninguna tarea. (No se realiza durante un tiempo 
     minimo sino solo el tiempo necesario (ej. en cuanto detecta a un rival va a por él inmediatamente).*/
    else {
      turn(); 
      leds(128,0,0);
    }
  }
}


/*Función asociada a interrupción externa del pulsador por flanco de bajada*/
void buttonPressed(){
  started=1; 
}

/*Función de medida de ultrasonidos (ver hoja de características)*/
void trig(){
  if(!triggered){
    digitalWrite(trigger,HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger,LOW);
    triggered = 1;
  }
}

/*Medida del tiempo de ida y vuelta del haz ultrasónico. Asociado a interrupción externa por cambio*/
void echoed(){
  static long utime;  
  if(digitalRead(echo) == HIGH){ // Cambio a nivel alto-> se inicia la medida
    utime =micros(); //Almacenmiento el tiempo actual
  }
  else{ //Cambio a nivel bajo -> se termina la medida
    ustime = abs(micros()-utime); //Diferencia de tiempos
    triggered =0; //Flag a 0 para volver a medir
  }
}

/* Efecto fade con el LED RGB*/
void fade(){
  static unsigned long ptime;
  static int color = red;
  static int value = 250;
  static int delta = -5;
  if((millis()-ptime)>FADE_TIME){
    if(value == 0){
      switch(color){
      case 9:
        color = green;
        break;
      case 10:
        color = blue;
        break;
      case 11:
        color = red;
        break;
      }
    }
    if(value == 255 || value == 0){
      delta  = -delta;
    }
    value += delta;
    analogWrite(color, value);
    ptime = millis();
  }
}

/* Cuenta atrás para el comienzo del combate*/
void countdown(){
  analogWrite(red, 0);
  analogWrite(green, 0);
  for(int i = 0; i < 5; i++){
    analogWrite(blue, 255);
    delay(500);
    analogWrite(blue, 0);
    delay(500);
  }
}

void reverse(){
  digitalWrite(IN1,HIGH);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN4,LOW);
}

void forward(){
  digitalWrite(IN2,HIGH);
  digitalWrite(IN4,HIGH);
  digitalWrite(IN1,LOW);
  digitalWrite(IN3,LOW);
}

void turn(){
  analogWrite(IN1,searchdir ? 128:0);
  digitalWrite(IN4,searchdir? HIGH:LOW);
  digitalWrite(IN2,searchdir? LOW:HIGH);
  analogWrite(IN3,searchdir ? 0:128);
}

void leds( byte r, byte g, byte b){
  analogWrite(red,r); 
  analogWrite(green,g); 
  analogWrite(blue,b); 
}



