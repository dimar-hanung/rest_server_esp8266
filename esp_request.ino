  #include <string.h>
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecure.h>
  #include <WiFiUdp.h>

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>

  // nyambung wifi
  char ssid[] = "MEIDA";  //  SSID WIFI
  char password[] = "vitoganteng";  // password WIFI
  
  // variable ultrasonik
  const int trigPin = 2;  //D4
  const int echoPin = 0;  //D3
  const int pompa   = 14;
  
  // defines variables ultrasionik
  long duration;
  int distance;
  
  //variabel soil 
  int sense_Pin = 0;  // Soil Sensor input at Analog PIN A0
  int value = 0;
  String pstatus;
  //fungsi i2c
  LiquidCrystal_I2C lcd(0x27, 16, 2);
  
  void setup()
  {
    Serial.begin(9600);
    lcd.begin();
  
    //memulai koneksi
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
  
    Serial.print("Connecting Wifi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    //selama wifi tidak tersambung menjalankan perintah while ini
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      lcd.setCursor(0, 0);
      lcd.print("Menyambung");
      lcd.setCursor(0, 1);
      lcd.print("ke wifi..");
      delay(500);
    }

    //ketika sudah tersambung
    Serial.println("");
    Serial.println("WiFi tersambung");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    //mendefinisikan sebagai input dan output pin ultrasonnik
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  
    pinMode(pompa, OUTPUT);
    delay(2000);
  }
  
  void loop()
  {
    //xx
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
  
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
  
    // Calculating the distance
    distance = duration *0.034 / 2;
    // Prints the distance on the Serial Monitor
    Serial.println(distance);
    
    //batas 
    Serial.print("MOISTURE LEVEL : ");
    value = analogRead(sense_Pin);
    value = value / 10;

    
    lcd.setCursor(0, 0);
    lcd.print("Lembab,Air :");
    lcd.setCursor(0, 1);
    lcd.println(String(value) + "," + String(distance));

//    if(distance<5){
//      digitalWrite(pompa, HIGH);
//    }else
//    {
//      digitalWrite(pompa, LOW);
//    }


    HTTPClient http;
  http.begin("http://ec2-18-208-181-162.compute-1.amazonaws.com/restapi8/api/product/read_one.php?id=60");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.GET();
  String payload = http.getString();
  http.writeToStream(&Serial);
    if(httpCode == 200)
  {
    // Allocate JsonBuffer
    // Use arduinojson.org/assistant to compute the capacity.
    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
    DynamicJsonBuffer jsonBuffer(capacity);
  
   // Parse JSON object
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      Serial.println(F("Parsing failed!"));
      return;
    }
  
    // Decode JSON/Extract values
    Serial.println(F("Response:"));

    pstatus = root["name"].as<char*>();
    
    Serial.println(pstatus);
    Serial.println(root["price"].as<char*>());

  }else
  {
    Serial.println("Error in response");
  }

  http.end();
  
  if(pstatus=="LOW"){
  digitalWrite(pompa, LOW);
  }else{
    digitalWrite(pompa, HIGH);
  }
    String json = "{\"id\":\"60\",\"name\":"+String(pstatus)+",\"price\":"+String(distance)+",\"description\":\"gps\",\"category_id\":\"1\",\"created\":\"gps\"}";
      

  http.begin("http://ec2-18-208-181-162.compute-1.amazonaws.com/restapi8/api/product/update.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.POST(json);
  http.writeToStream(&Serial);
  http.end();
  

  
    delay(1500);
  }
