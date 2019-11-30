#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h> 
#include <WiFiClient.h> 
#include <SoftwareSerial.h>
//#include <PubSubClient.h> 
#include <ThingSpeak.h>  
#define myPeriodic 15 //in sec | Thingspeak pub is 15sec
#define ONE_WIRE_BUS D7  // DS18B20 on arduino pin2 corresponds to D4 on physical board
#define ONE_WIRE_BUS1 D5 //motor tempearture
#define select_line D6
const char* ssid = "iPhone";  
const char* password = "amitgg1675";
WiFiClient client;  
OneWire twoWire(ONE_WIRE_BUS1);
DallasTemperature DS18B20(&twoWire);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
unsigned long myChannelNumber = 456710;  
const char * myWriteAPIKey = "DYUU1BOU3K0DYGOB";  
const char * myReadAPIKey = "C6LKS8G8R74ORDT3";
unsigned long motorChannelNumber = 461950;  
const char * motorWriteAPIKey = "IDSFSO3O2E56JZ42";  
const char * motorReadAPIKey = "2I781BE8NDC7GPNB";


uint8_t mtemp, mtemp1; 
uint8_t wtemp; 
const int analogInPin = A0; 
int sensorValue = 0; 
unsigned long int avgValue; 
float b;
int buf[10],temp;
float pHVol, phValue;
uint8_t turbidityValue,turbidityV; 
int data;
SoftwareSerial mySerial(D1, D2);  //(Rx,Tx)
char msg;
const int relay = D8;
const int trigPin = D3;
const int echoPin = D0;
// defines variables
long duration;
int distance;
int level;
boolean mot_man, flag=0, mot_auto, mot_final;




//Individual flags for sending message
int flag2=0;
int flag3=0;
int flag4=0;
int flag5=0;
int flag6=0;
int relay_state=0;
int state=0;
unsigned long start=millis();



void setup()  
{  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(relay, OUTPUT);
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600);   
  delay(10);  
  
  // Connect to WiFi network  
  Serial.println();  
  Serial.println();  
  Serial.print("Connecting to ");  
  Serial.println(ssid);  
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED)  
  {  
   delay(500);  
   Serial.print(".");  
  }  
  Serial.println("");  
  Serial.println("WiFi connected");  
  // Print the IP address  
  Serial.println(WiFi.localIP());  
  ThingSpeak.begin(client);  
  ThingSpeak.writeField(motorChannelNumber,1,1, motorWriteAPIKey);
  

}  




void loop()   
{  
  Motor_Temp();
  Water_Temp();
  Ph();
  Turbidity();
  Water_Level();
  if(mtemp>=mtemp1+2 || phValue<5 || phValue>8 || distance>18 || turbidityV>2) {
    mot_auto=0;
    ThingSpeak.setField(1, mtemp); 
    ThingSpeak.setField(2, wtemp); 
    ThingSpeak.setField(3, phValue); 
    ThingSpeak.setField(4, turbidityV); 
    ThingSpeak.setField(5, distance); 
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  }
  else mot_auto=1;
  ThingSpeak.writeField(motorChannelNumber, 2, mot_auto, motorWriteAPIKey);
  mot_final=(mot_man && mot_auto);
  Serial.print("manual ");  Serial.println(mot_man);
  Serial.print("auto ");  Serial.println(mot_auto);
  Serial.print("final ");  Serial.println(mot_final);
  if (mot_final == 0) {
    digitalWrite(relay, HIGH);
    Serial.println("motor off");
  }
  else {
    digitalWrite(relay, LOW);
    Serial.println("motor on");
  }
  ThingSpeak.writeField(motorChannelNumber, 3, mot_final, motorWriteAPIKey);

  if(millis()-start>15000)  //delay of 15 .seconds
  {  
     Serial.print("Motor Temperature Value is :");  
     Serial.print(mtemp);  
     Serial.println("C"); 
     ThingSpeak.setField(1, mtemp); 
     
     Serial.print("Water Temperature Value is :");  
     Serial.print(wtemp);  
     Serial.println("C");    
     ThingSpeak.setField(2, wtemp); 

     Serial.print("Ph Value is :");  
     Serial.print(phValue);   
     Serial.print("\n"); 
     ThingSpeak.setField(3, phValue); 

     Serial.print("Turbidity Value is :");  
     Serial.print(turbidityV);  
     Serial.print("\n"); 
     ThingSpeak.setField(4, turbidityV); 

     Serial.print("Distance Value is :");  
     Serial.println(distance);
     Serial.print("\n");
     ThingSpeak.setField(5, distance); 
     
     ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

     Message();
     
     start=millis();
  }
}

void Motor_Temp()
{
  DS18B20.requestTemperatures(); 
  mtemp = DS18B20.getTempCByIndex(0);
  mot_man=ThingSpeak.readFloatField(motorChannelNumber,1, motorReadAPIKey);
  if(flag==0) {
  mtemp1=mtemp;
  flag=1;}
}

void Water_Temp()
{
  sensor.requestTemperatures(); 
  wtemp = sensor.getTempCByIndex(0);
}

//analog sensor to digital
void Ph()
{
  digitalWrite(select_line, 0);
  delay(10);
  for(int i=0;i<10;i++) 
  { 
    buf[i]=analogRead(analogInPin);
    delay(10);
  }
  for(int i=0;i<9;i++)
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
          temp=buf[i];
          buf[i]=buf[j];
          buf[j]=temp;
      }
    }
  }
  avgValue=0;                                   
  for(int i=2;i<8;i++)
  avgValue+=buf[i];
  pHVol=(float)avgValue*5.0/1024/6;
  phValue = 2.1875 * pHVol/1.5;
}

//Analog sensor
void Turbidity()
{
  digitalWrite(select_line, 1);
  delay(10); 
  turbidityValue = analogRead(A0);
  turbidityV = turbidityValue/100;
}

//Ultrasonic sensor
void Water_Level()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance= duration*0.034/2;
}

void Message()
{

  //Set flags back if value is normal
     if(flag2==1 && mtemp<120)
     flag2=0;

     if(flag4==1 && (phValue>5 || phValue<8))
     flag4=0;

     if(flag5==1 && turbidityV<2)
     flag5=0;

     if(flag6==1 && distance>18)
     flag6=0;
     
     String test="Exceeded threshold value(s): ";
     String str="Exceeded threshold value(s): ";
     if(mtemp>=mtemp1+2 && flag2==0)
     {  flag2=1;
        str+="Motor temp ";
     }
     if((phValue<5 || phValue>8) && flag4==0)
     {  flag4=1;
        str+="pH ";
     }
     if(turbidityV>2 && flag5==0)
     {  flag5=1;
        str+="Turbidity ";
     }
     if(distance>18 && flag6==0)
     {  flag6=1;
        str+="Water level ";
     }
     
     if(str!=test && (flag2==1 || flag3==1 || flag4==1 || flag5==1 || flag6==1))
     SendMessage(str);
     
}

void SendMessage(String str)
{ 
  Serial.println("send msg");
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+919654660103\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.println(str);// The SMS text you want to send
  delay(100);
  mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  Serial.println("msg sent");
}
