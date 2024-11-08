#include <WiFi.h>
#include <HTTPClient.h>

// ข้อมูล WiFi และ ThingsBoard
String ssid = "Wokwi-GUEST";
String password = "";

String tbHost = "thingsboard.cloud";
String tbToken = "n5ksm170z9xuwo65mz45"; // Access Token ของ ThingsBoard

// LINE Messaging API
String lineAccessToken = "J8C//G7Lmm4gWLytK4f6dPsweAnI24NYSCslov+Qv49XVXuzzb+ZBu31P+sJy/83Zke1uEUK0Pxb4ZwKnsUC63+rOF0VOZTmOuAe76jv38UTy7gLAETTeBO+KHn+kPVC9I22hbp44rQKtvOJEZKxvgdB04t89/1O/w1cDnyilFU="; // นำ Token จาก LINE Channel ของคุณมาใส่

int analogInPin = 34;  
int sensorValue;
float voltage;
float bat_percentage;

// ตัวต้านทานที่ใช้ในการแบ่งแรงดัน
float R1 = 220000.0; // 220k ohm
float R2 = 10000.0; // 10k ohm

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  // รอจนกว่า WiFi จะเชื่อมต่อสำเร็จ
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  delay(1500); 
}

void loop() {
  // อ่านค่าแรงดันจากเซ็นเซอร์
  sensorValue = analogRead(analogInPin);
  voltage = (sensorValue * 3.3) / 4095.0; // สำหรับ ESP32 ที่มีค่า ADC สูงสุด 4095
  bat_percentage = voltage / (R2 / (R1 + R2));
  
  if (bat_percentage >= 100) {
    bat_percentage = 100;
  }
  if (bat_percentage <= 0) {
    bat_percentage = 0;
  }

  Serial.print("Analog Value = ");
  Serial.print(sensorValue);
  Serial.print("\t Output Voltage = ");
  Serial.print(voltage);
  Serial.print("\t Battery Percentage = ");
  Serial.println(bat_percentage);

  //************************ThingsBoard************************
  String serverName = "http://" + tbHost + "/api/v1/" + tbToken + "/telemetry";

  // ส่งค่าแรงดันไปยัง ThingsBoard
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"percentage\": " + String(bat_percentage) + 
                      ", \"Analog_Value\": " + String(sensorValue) + 
                      ", \"Output_Voltage\": " + String(voltage) + "}";

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.println("Data sent successfully!");
    } else {
      Serial.print("Error sending data: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }

  // ส่งแจ้งเตือนผ่าน LINE Messaging API
  String message = "Battery Status: " + String(bat_percentage) + 
                   "%, Voltage: " + String(voltage) + "V";
  sendLineMessage(message);

  delay(10000); // ส่งข้อมูลทุกๆ 10 วินาที
}

void sendLineMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.line.me/v2/bot/message/broadcast");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + lineAccessToken);

    // ข้อความที่ต้องการส่ง
    String jsonData = "{\"messages\":[{\"type\":\"text\",\"text\":\"" + 
                        message + "\"}]}";

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.println("Message sent to LINE successfully!");
    } else {
      Serial.print("Error sending message to LINE: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}