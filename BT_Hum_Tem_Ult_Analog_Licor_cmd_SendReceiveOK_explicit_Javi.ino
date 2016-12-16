/*
 * Experimental prototype for measuring stations
 * =============================================
 * 
 * Company: ITER-INVOLCAN
 * Autor: Aarón Pérez Martín
 * Email: aaperez@iter.es
 * Year : 2016
 * Version : 1.0
 */
//Libs
#include <DHT.h>

//Connections Notes:
//==================
//BluePins must be inverted
//UltPin must be inverted

//pins
#define LEDPIN 13
#define LED2PIN 12
#define ULTPIN_TX 8 //inverted connections
#define ULTPIN_RX 7
#define HUMPIN 9

//types
#define TRAMA "[Temperatura,Humedad,Distancia,SalidasAnalogicas,Licor]"
#define DHTTYPE DHT11 //HUMPIN
DHT dht(HUMPIN,DHTTYPE);

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
    Serial2.begin(9600);//Bluetooth
    Serial3.begin(9600);//Licor sensor
    
    while(!Serial){;}
    Serial.flush();
    Serial.println("Arduino is loading...");
    
    while(!Serial2){;}
    Serial2.flush();
    Serial.println("Serie 2: Bluetooth device ready.");

    while(!Serial3){;}
    Serial3.flush();
    Serial.println("Serie 3: Licor sensor ready.");

    Serial.println("Arduino ready.");

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
    Serial2.flush();
    Serial3.flush();
    delay(200);

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
  //licorData = "<co2>394.76</co2><tem>51.2</tem><pre>101383</pre><h2o>394.76</h2o><bat>12.4</bat>";
  
  //if(Serial3.available() > 0){
      //Se captura sin tratamiento para el aplique de formulas en el software posterior
      licorData = Serial3.readStringUntil('\n');
  //}
}

void receiveDataBT(){
  while(Serial2.available()>0){
    char inByte = Serial2.read();
    switch (inByte) {
      case '1':
        digitalWrite(LED2PIN, HIGH);
        Serial.println("Turn on LED");
        break;
      case '0':
        digitalWrite(LED2PIN, LOW);
        Serial.println("Turn off LED");
        break;
    }
   }
}

void sendDataBT(){
  generateDataToSend();
  
  //To Bluetooth serial port
  if(Serial2.available()){
    Serial2.println(TRAMADATA);
  }
  
  //To Monitor serial port
  Serial.println(TRAMADATA);
}

void generateDataToSend(){

  String temp = String(temData);
  String humi = String(humData);
  String dist = String(distUltData);
  String licor = String(licorData);
  
  if((temp.length()>0) &&
    (humi.length()>0) &&
    (dist.length()>0) &&
    (licor.length()>0)){
      
    TRAMADATA = "[";
    TRAMADATA += "TAM:" + temp + ",";
    TRAMADATA += "HAM:" + humi + ",";
    TRAMADATA += "DIS:" + dist + ",";
    TRAMADATA += analogData + ",";
    TRAMADATA += "LIC:" + licor;
    TRAMADATA += "]";
  }

}

void clearValues(){
  //This method is necessary to read properly the values

  //General Variables
  humData=0;
  temData=0;
  timeUltData=0;
  distUltData=0;

  licorData = "";
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

