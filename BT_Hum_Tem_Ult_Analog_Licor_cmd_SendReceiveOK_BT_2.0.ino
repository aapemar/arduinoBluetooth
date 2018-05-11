/*
 * Experimental prototype for measuring stations
 * =============================================
 * 
 * COMPANY: ITER
 * AUTHOR: Aarón Pérez Martín
 * EMAIL: aaperez@iter.es
 * YEAR : 2016
 * VERSION : 1.0
 */
 
//Libs
#include <DHT.h>

//Connections Notes:
//==================
//BluePins must be inverted
//UltPin must be inverted

//pins
#define LEDPIN 13   // activity led

#define COMPIN_1 12   //Actuator 01
#define COMPIN_2 11   //Actuator 02
#define COMPIN_3 10   //Actuator 03

#define HUMPIN 9
#define ULTPIN_TX 8 //inverted connections
#define ULTPIN_RX 7


#define HUMSOILPIN 6

//types
#define TRAMA "[Temperatura,Humedad,Distancia,SalidasAnalogicas,Licor]"
#define DHTTYPE DHT22 //HUMPIN
DHT dht(HUMPIN,DHTTYPE);

//for sensor operations
float humData;
float temData;
long timeUltData;
int distUltData;
String analogData;
String licorData;

//for analog pins
int const maxPin = 4;
float sensorValue[maxPin];
float voltageValue[maxPin];
int defaultValue = 0;

//for dataToSend
String TRAMADATA;

//For Comunication BT-Arduino
String patternBT="+COMMAND:";

int maxTags = 6;

//For XML parser
char *startTags[] = { "<celltemp>", "<cellpres>", "<co2>", "<co2abs>", "<ivolt>", "<raw>"  };
char *nameTags[] = { "celltemp", "cellpres", "co2", "co2abs", "ivolt", "raw"  };
char *endTags[] = { "</celltemp>", "</cellpres>", "</co2>", "</co2abs>", "</ivolt>", "</raw>"  };

void setup()
   {
    Serial.begin(9600);//Monitor
    Serial2.begin(9600);//Bluetooth
    Serial3.begin(9600);//Licor sensor
    Serial.println("");
    
    while(!Serial){;}
    Serial.flush();
    Serial.println("Arduino is loading...");
    
    while(!Serial2){;}
    Serial2.flush();
    //delay(10);
    //Serial2.println("At+NAME=BLUE");
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

    //For Commands
    pinMode(LEDPIN,OUTPUT);
    pinMode(COMPIN_1,OUTPUT);
    pinMode(COMPIN_2,OUTPUT);
    pinMode(COMPIN_3,OUTPUT);
    delay(10);
    digitalWrite(COMPIN_1, HIGH);
    digitalWrite(COMPIN_2, HIGH);
    digitalWrite(COMPIN_3, HIGH);
    delay(1000);
    digitalWrite(COMPIN_1, LOW);
    digitalWrite(COMPIN_2, LOW);
    digitalWrite(COMPIN_3, LOW);
   }

void loop(){
    //delay(20);

    //Activamos las mediciones
    digitalWrite(LEDPIN,HIGH);

    clearValues();
    clearAnalogValues();
    readLicor();
    readHum();
    readUlt();
    readAnalog();
       
    sendDataBT();
    
    //Receive data from BT
    receiveDataBT();

    digitalWrite(LEDPIN, LOW);
}

//Methods
void readHum(){
  humData = dht.readHumidity();
  temData = dht.readTemperature();
  if(isnan(humData))
    humData = 0;
  if(isnan(temData))
    temData = 0;  
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
  if(isnan(timeUltData))
    timeUltData = 0;
  else
    distUltData = microsecondsToCentimeters(timeUltData);
}

void readAnalog(){
  String s = "[";
  for(int i = 0;i<maxPin; i++){
    
    sensorValue[i] = analogRead(i);
    //Conversion of voltage
    voltageValue[i] = ((sensorValue[i]/1023)*5);

    if(i<10)
      s = s + "A0" + String(i) + "|" + voltageValue[i];
    else
      s = s + "A" + String(i) + "|" + voltageValue[i];
      
    if(i<maxPin -1)
      s += "_"; 
  }
  s += "]";
  analogData = s;
}

void readLicor(){
  //Original data
  //<li820><data><celltemp>5.1704649e1</celltemp><cellpres>1.0152190e2</cellpres><co2>4.0337109e2</co2><co2abs>6.4475774e-2</co2abs><ivolt>1.4199829e1</ivolt><raw>3787568,3639155</raw></data></li820>

  licorData = ""; //"[]"
  //delay(50);
  //Serial3.flush();
  String tempData = Serial3.readStringUntil('\r');
  delay(5);
  //Serial.println(tempData);
  //Serial3.flush();
  readTags(tempData);
  delay(5);

  //To avoid problems with the chunk form Licor
  if(licorData.indexOf('<')>0 || licorData.indexOf('>')>0)
    licorData = ""; //"[]"
}

/*
 * Read de XML data from de licor sensor and we get the values using given tags
 */
void readTags(String originalData)
{
  delay(5);
  //String originalData = licorData;
  int count = -1;
  String tempData = "[]";
  //Serial.println(originalData);
  for (int i=0; i<maxTags; i++ )
  {
     int startTag = originalData.indexOf(startTags[i]);
     int endTag = originalData.indexOf(endTags[i]);
  
     int lengthTag = strlen(startTags[i]);
     String item = originalData.substring(startTag + lengthTag, endTag);
     String tag = nameTags[i];
     
     count++;
     if(count == 0){
        tempData = "[";
     }
          
     if(item.length() > 0){
        item.replace(",", ".");
        if(i < maxTags -1)
          tempData += tag + "|" + item + "_";
        else
          tempData += tag + "|" + item;
     }   
     if(i == maxTags -1 )
        tempData += "]";
        
  }
  //Serial.println(tempData);
  //To avoid problems with the chunk form Licor. Next chars mean the chunk is corrupted 
  //if(tempData.indexOf('<')>0 || tempData.indexOf('>')>0)
  //tempData = "[]";
    
  licorData = String(tempData);
  //Serial.println(licorData);
}

void receiveDataBT(){
  while(Serial2.available()>0){
    /*
    char inByte = Serial2.read();
    //Serial.print("Reading.. ");    
    Serial.println(inByte);*/

    String chunk = Serial2.readStringUntil('\n');
    //Serial.print(chunk);
    if(chunk.startsWith(patternBT)){
      Serial.println("He recibido un comando");
      int indexStart = chunk.indexOf(patternBT) + patternBT.length();
      int finalStart = indexStart + 1; //chunk.length();
      String commando = chunk.substring(indexStart,finalStart); //finalStart -1

      Serial.println(commando);
      
      if(commando.equals("0")){
         digitalWrite(COMPIN_1, HIGH);
         Serial.println("Actuator 01 ON");
      }else if(commando.equals("1")){
        digitalWrite(COMPIN_1, LOW);
        Serial.println("Actuator 01 OFF");
        
      }else if(commando.equals("2")){
        digitalWrite(COMPIN_2, HIGH);
        Serial.println("Actuator 02 ON");
      }else if(commando.equals("3")){
        digitalWrite(COMPIN_2, LOW);
        Serial.println("Actuator 02 OFF");

      }else if(commando.equals("4")){
        digitalWrite(COMPIN_3, HIGH);
        Serial.println("Actuator 03 ON");
      }else if(commando.equals("5")){
        digitalWrite(COMPIN_3, LOW);
        Serial.println("Actuator 03 OFF");

        
      }else if(commando.equals("9")){
        digitalWrite(COMPIN_1, LOW);
        delay(500);
        digitalWrite(COMPIN_2, LOW);
        delay(500);
        digitalWrite(COMPIN_3, LOW);
        Serial.println("All Actuators OFF");
        
      }else{
        Serial.println("Received data:" + chunk);
        Serial.println("Unknown command!:" + commando);
      }
    }
  } 
}

void sendDataBT(){
  generateDataToSend();
  
  //To Bluetooth serial port
  Serial2.println(TRAMADATA);
  
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
    TRAMADATA += "ANA:" + analogData + ",";
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
  licorData="";
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


