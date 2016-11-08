
//Libs
//#include <SoftwareSerial.h>
#include <DHT.h>

//pins
#define LEDPIN 13
#define LED2PIN 12
/*#define BLUPIN_TX 1 //inverted connections
#define BLUPIN_RX 0*/
#define ULTPIN_TX 8 //inverted connections
#define ULTPIN_RX 7
#define HUMPIN 9

//types
#define TRAMA "[Temperatura,Humedad,Distancia,Tiempo,SalidasAnalogicas,Licor]"
#define DHTTYPE DHT11 //HUMPIN
DHT dht(HUMPIN,DHTTYPE);
//SoftwareSerial BT1(BLUPIN_TX,BLUPIN_RX); // RX, TX recorder que se cruzan

//for sensor operations
float humData;
float temData;
long timeUltData;
int distUltData;
String analogData;
String licorData;

//for analog pins
int const maxPin = 16;
float sensorValue[maxPin];
float voltageValue[maxPin];
int defaultValue = 0;

//for dataToSend
String TRAMADATA;

void setup()
   {
    Serial.begin(9600);//Monitor
    while(!Serial){;}
    Serial.flush();
    Serial.println("Arduino ready.");

    Serial1.begin(9600);//Licor sensor
    while(!Serial1){;}
    Serial1.flush();
    Serial.println("Serie 1: Licor sensor ready.");
      
    Serial2.begin(9600);//Bluetooth
    while(!Serial2){;}
    Serial2.flush();
     Serial.println("Serie 2: Bluetooth device ready.");

    Serial.println("");
    Serial.println("Formato trama");
    Serial.println(TRAMA);
      
    //For humidity sensor
    dht.begin();
    
    //For ultrasonic sensor
    pinMode(ULTPIN_TX, OUTPUT);
    pinMode(ULTPIN_RX, INPUT);

    pinMode(LEDPIN,OUTPUT);
    pinMode(LED2PIN,OUTPUT);

    digitalWrite(LED2PIN, LOW);
    
   }

void loop(){

    Serial.flush();

    //Activamos las mediciones
    digitalWrite(LEDPIN,HIGH);

    clearValues();
    clearAnalogValues();
    readHum();
    readUlt();
    readAnalog();
    readLicor();
    
    sendDataBT();

    digitalWrite(LEDPIN, LOW);
    //Receive data from BT??
    receiveDataBT();
    
}

//Methods
void readHum(){
  humData = dht.readHumidity();
  temData = dht.readTemperature();
  //delay(200);
}

void readUlt(){
  //Clean pins ultrasonic
  digitalWrite(ULTPIN_TX, LOW);
  delayMicroseconds(5);
  digitalWrite(ULTPIN_TX, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTPIN_TX, LOW);

  //Read the returned wave
  pinMode(ULTPIN_RX, INPUT);
  timeUltData = pulseIn(ULTPIN_RX, HIGH);
  distUltData = microsecondsToCentimeters(timeUltData);
  //delay(200);
}
void readAnalog(){
  String s = "";//"Analog Values [";
  for(int i = 0;i<maxPin; i++){
    
    sensorValue[i] = analogRead(i);
    //Conversion of voltage
    voltageValue[i] = ((sensorValue[i]/1023)*5);

    if(i<10)
      s = s + "A0" + String(i) + ":" + voltageValue[i];
    else
      s = s + "A" + String(i) + ":" + voltageValue[i];
      
    if(i<maxPin -1)
      s=s+"," ; 
  }
  //s=s+"]";
  analogData = s;
}

void readLicor(){
  licorData = "<co2>394.76</co2><tem>51.2</tem><pre>101383</pre><h2o>394.76</h2o><bat>12.4</bat>";
  
  /*if(Serial1.available() > 0){
      //Se captura sin tratamiento para el aplique de formulas en el software posterior
      licorData = Serial1.readStringUntil('\n');
  }*/
  
}
/*
String getLineLicor(){
  licorData = "";
  String S = "" ;
  char message[1000];
  if (Serial1.available()){
    char c = Serial1.read(); ;
        while ( c != '\n')            //Hasta que el caracter sea intro
          {     S = S + c ;
                strcat(message,c);
                //delay(25) ;
                c = Serial1.read();
          }
        licorData = licorData + S;
        return(licorData);
  }
}*/

void receiveDataBT(){
  while(Serial2.available()>0){
    char inByte = Serial2.read();
    switch (inByte) {
      case '1':
        digitalWrite(LED2PIN, HIGH);
        Serial.println("Turn on LED");
        break;
      case '2':
        digitalWrite(LED2PIN, LOW);
        Serial.println("Turn off LED");
        break;
    }
   }
}

void sendDataBT(){
  generateDataToSend();
  
  //To Bluetooth serial port
  //if(Serial2.available()){
    Serial2.println(TRAMADATA);
  //}
  
  //To Monitor serial port
  Serial.println(TRAMADATA);
}

void generateDataToSend(){
/*
  if(strcmp(String(temData),"") &&
    strcmp(String(humData),"") &&
    strcmp(String(distUltData),"") &&
    strcmp(String(timeUltData),"") &&
    strcmp(analogData,"") &&
    strcmp(licorData,""))
    return false;
  */
  TRAMADATA = "[";
  TRAMADATA += "TAM:" + String(temData) + ",";
  TRAMADATA += "HAM:" + String(humData) + ",";
  TRAMADATA += "DIS:" + String(distUltData) + ",";
  // TRAMADATA += String(timeUltData) + "},{";
  TRAMADATA += analogData + ",";
  TRAMADATA += "LIC:" + licorData;
  TRAMADATA += "]";

  //return TRAMADATA;
}

void clearValues(){
  //This method is necessary to read properly the values

  //General Variables
  humData=0;
  temData=0;
  timeUltData=0;
  distUltData=0;
}

void clearAnalogValues(){
  //Set all values to defaul value
  for(int i = 0;i<maxPin; i++){
    sensorValue[i] = defaultValue;
  } 
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}


