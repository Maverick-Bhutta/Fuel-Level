//Simplest way to use firebase realtime database for display /control fron anyware your sensors / things over internet (IOT)
// Backend is Google firebase Realime Databas and front end is web page hosted in my gethub account
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Preferences.h>
#include <DHT11.h>
// Create an instance of the DHT11 class.
// - For Arduino: Connect the sensor to Digital I/O Pin 2.
// - For ESP32: Connect the sensor to pin GPIO2 or P2.
// - For ESP8266: Connect the sensor to GPIO2 or D4.
DHT11 dht11(2);
Preferences prefs;
// prefrence is new fuction for ESP32 EEPROM red/write  methods
const char* PREF_NAMESPACE = "wifi";
const char* PREF_SSID_KEY = "ssid";
const char* PREF_PASS_KEY = "pass";
unsigned long sendDataPrevMillis = 0;
unsigned long connectTimeoutMs = 15000; // 15 seconds
/* 1. Define the WiFi credentials */
//#define WIFI_SSID "***********"
//#define WIFI_PASSWORD "**********"

/* 2. Define the API Key */
#define API_KEY "*****************************"

/* 3. Define the RTDB URL */
#define DATABASE_URL "**************************" 

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "******************"
#define USER_PASSWORD "**************"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
//Configure GPIO of esp device to get distance and dht sensor data
//const int ledPin = 2;
const int Trig = 32;
const int echo = 35;
float flevel = 0.0;
int Temperature = 0;
int Humidity = 0;

String readLineFromSerial(unsigned long timeout = 0) {
  String s;
  unsigned long start = millis();
  while (true) {
    while (Serial.available()) {
      char c = (char)Serial.read();
      if (c == '\r') continue;
      if (c == '\n') {
        return s;
      }
      s += c;
    }
    if (timeout && (millis() - start >= timeout)) break;
    delay(10);
  }
  return s;
}
bool connectWiFi(const char* ssid, const char* pass, unsigned long timeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(100);
  WiFi.begin(ssid, pass);

  unsigned long start = millis();
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start >= timeoutMs) {
      Serial.println("\nConnection timed out.");
      return false;
    }
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
  
  connect_FBcloud();
  return true;
}
void saveCredentials(const String& ssid, const String& pass) {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.putString(PREF_SSID_KEY, ssid);
  prefs.putString(PREF_PASS_KEY, pass);
  prefs.end();
  Serial.println("Credentials saved to Preferences.");
}
bool loadCredentials(String& ssid, String& pass) {
  prefs.begin(PREF_NAMESPACE, true); // read-only
  ssid = prefs.getString(PREF_SSID_KEY, "");
  pass = prefs.getString(PREF_PASS_KEY, "");
  prefs.end();
  return ssid.length() > 0;
}
void clearCredentials() {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.remove(PREF_SSID_KEY);
  prefs.remove(PREF_PASS_KEY);
  prefs.end();
  Serial.println("Saved credentials cleared.");
}
bool connect_FBcloud(){
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
  return true;
}
void promptAndStoreCredentials() {
  Serial.println();
  Serial.println("Enter WiFi SSID (or type 'clear' to erase saved credentials):");
  String ssid = readLineFromSerial();
  ssid.trim();
  if (ssid.equalsIgnoreCase("clear")) {
    clearCredentials();
    return;
  }
  if (ssid.length() == 0) {
    Serial.println("No SSID entered. Aborting input.");
    return;
  }

  Serial.println("Enter WiFi Password (leave empty for open networks):");
  String pass = readLineFromSerial();
  pass.trim();

  saveCredentials(ssid, pass);
  Serial.print("Attempting to connect to '");
  Serial.print(ssid);
  Serial.println("' ...");
  if (!connectWiFi(ssid.c_str(), pass.c_str(), connectTimeoutMs)) {
    Serial.println("Failed to connect with provided credentials.");
  }
}
void setup()
{
  //***********Connect and configure WIFI network credential first *********//
  Serial.begin(115200);
  //pinMode(ledPin, OUTPUT);
  pinMode(Trig,OUTPUT);
  pinMode(echo,INPUT);
  //digitalWrite(ledPin, LOW);
  while (!Serial) { delay(10); } // wait for Serial on some boards
  Serial.println();
  Serial.println(" WiFi module Setup");

  String ssid, pass;
  if (loadCredentials(ssid, pass)) {
    Serial.print("Found saved SSID: ");
    Serial.println(ssid);
    Serial.println("Trying to connect...");
    if (!connectWiFi(ssid.c_str(), pass.c_str(), connectTimeoutMs)) {
      Serial.println("Auto-connect failed. Type 'setup' to enter new credentials or 'clear' to erase saved credentials.");
    } else {
      Serial.println("WiFi connected using saved credentials.");
    }
  } else {
    Serial.println("No saved WiFi credentials found. Type 'setup' to enter them now.");
    //promptAndStoreCredentials();
  }
  Serial.println();
  Serial.println("Available serial commands:");
  Serial.println("  setup  - enter new SSID/password and save");
  Serial.println("  clear  - erase saved credentials");
  Serial.println("  status - show WiFi status and IP");
  Serial.println();
}
void Measure_Distance(float & Distance){
  //Measure Distance with Ultrasonic Sensor
  digitalWrite(Trig,LOW);
  delayMicroseconds(2);
  digitalWrite(Trig,HIGH);
  //Give trigger pulse of 10uS
  delayMicroseconds(10);
  digitalWrite(Trig,LOW);
  //Measure the Tx/Rx time of echo pulse
  long Duration = pulseIn(echo,HIGH);
  Distance = Duration * 0.0343 / 2;
  //return Distance;
}
void Measure_Temp(){
  int result = dht11.readTemperatureHumidity(Temperature, Humidity);
  if (result == 0) {
        // Serial.print("Temperature: ");
        // Serial.print(temperature);
        // Serial.print(" °C\tHumidity: ");
        // Serial.print(humidity);
        // Serial.println(" %");
    } else {
        // Print error message based on the error code.
        Serial.println(DHT11::getErrorString(result));
    }
}
void loop()
{
  if (Serial.available()) {
    String cmd = readLineFromSerial();
    cmd.trim();
    if (cmd.equalsIgnoreCase("setup")) {
      promptAndStoreCredentials();
    } 
    else if (cmd.equalsIgnoreCase("clear")) {
      clearCredentials();
    } 
    else if (cmd.equalsIgnoreCase("status")) {
      wl_status_t st = WiFi.status();
      Serial.print("WiFi status: ");
      switch (st) {
        case WL_CONNECTED: Serial.println("CONNECTED"); break;
        case WL_NO_SSID_AVAIL: Serial.println("NO_SSID_AVAIL"); break;
        case WL_CONNECT_FAILED: Serial.println("CONNECT_FAILED"); break;
        case WL_IDLE_STATUS: Serial.println("IDLE_STATUS"); break;
        case WL_DISCONNECTED: Serial.println("DISCONNECTED"); break;
        default: Serial.println("UNKNOWN"); break;
      }
      if (st == WL_CONNECTED) {
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("RSSI: ");
        Serial.println(WiFi.RSSI());
      }
    } 
    else if (cmd.length() > 0) {
      Serial.println("Unknown command. Use 'setup', 'clear', or 'status'.");
    }
  }
  Measure_Temp();
  Measure_Distance(flevel);

  Serial.print("fuel level: ");
  Serial.print(flevel);
  Serial.println(" Gal");
  Serial.print("Temp / Humidity :   ");
  Serial.print(Temperature);
  Serial.print(" oC / ");
  Serial.println(Humidity );
  // Firebase.ready() should be called repeatedly to handle authentication tasks.
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    int State;
    // if(Firebase.RTDB.getInt(&fbdo, "/FuelTank/shutdown", &State)){
    //   digitalWrite(ledPin, State);
    // }else{
    //   Serial.println(fbdo.errorReason().c_str());
    // }
    Firebase.RTDB.setInt(&fbdo,"/FuelTank/Temp",Temperature);
    Firebase.RTDB.setInt(&fbdo,"/FuelTank/Humidity",Humidity);
    Firebase.RTDB.setFloat(&fbdo,"/FuelTank/Level",flevel);
    Serial.println(" Updated Firebase Database" );
  }
  
}
