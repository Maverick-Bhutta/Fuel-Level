//Simplest way to use firebase realtime database for display /control fron anyware your sensors / things over internet (IOT)
// Backend is oogle firebase Realime Databas and front end is web page hosted in my gethub account
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Rayyaan Ahmed"
#define WIFI_PASSWORD "ee2000arqam"

/* 2. Define the API Key */
#define API_KEY "AIzaSyBxI0n-LPaAn8dAW--RLWpczN6a01W9DKE"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://seven-star-ent-default-rtdb.asia-southeast1.firebasedatabase.app/" 

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "shaukathayat860@gmail.com"
#define USER_PASSWORD "Maverick860"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

const int ledPin = 2;
const int Trig = 32;
const int echo = 35;
float flevel;
float ftemp,fhumidity;
void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(Trig,OUTPUT);
  pinMode(echo,INPUT);

  digitalWrite(ledPin, LOW);

  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
}
float Measure_Distance(){
  //Measure Distance with Ultrasonic Sensor
  digitalWrite(Trig,LOW);
  delayMicroseconds(2);
  digitalWrite(Trig,HIGH);
  //Give trigger pulse of 10uS
  delayMicroseconds(10);
  digitalWrite(Trig,LOW);
  //Measure the Tx/Rx time of echo pulse
  long Duration = pulseIn(echo,HIGH);
  float Distance = Duration * 0.0343 / 2;
  return Distance;
}
void loop()
{
  flevel = Measure_Distance();
  Serial.print("flevel");
  Serial.print(flevel);
  Serial.println(" Gal");
  ftemp = 20.50;
  fhumidity = 82.55;

  // Firebase.ready() should be called repeatedly to handle authentication tasks.
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

  int State;
   if(Firebase.RTDB.getInt(&fbdo, "/FuelTank/shutdown", &State)){
    digitalWrite(ledPin, State);
   }else{
    Serial.println(fbdo.errorReason().c_str());
   }
    Firebase.RTDB.setFloat(&fbdo,"/FuelTank/Temp",ftemp);
    Firebase.RTDB.setFloat(&fbdo,"/FuelTank/Humidity",fhumidity);
    Firebase.RTDB.setFloat(&fbdo,"/FuelTank/Level",flevel);
  }
  
}