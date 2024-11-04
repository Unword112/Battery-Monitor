#include <WiFi.h>
#include <HTTPClient.h>

// ข้อมูล WiFi และ ThingsBoard
String ssid = "YOUR_WIFI_NAME";
String password = "YOUR_WIFI_PASSWORD";

String tbHost = "thingsboard.cloud";  // เซิร์ฟเวอร์ของ ThingsBoard
String tbToken = "YOUR_TOKEN"; // Access Token ของ ThingsBoard

int analogInPin  = 34;  
int sensorValue; 
float voltage;
float bat_percentage;

void setup() {

  Serial.begin(9600);
  WiFi.begin(ssid, password);

  // รอจนกว่า WiFi จะเชื่อมต่อสำเร็จ
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  delay(1500); 
}

void loop() {
 
  sensorValue = analogRead(analogInPin);
  voltage = (sensorValue * 3.3) / 4095.0;  // สำหรับ ESP32 ที่มีค่า ADC สูงสุด 4095
  bat_percentage = mapfloat(voltage, 10, 16.8, 0, 100); // แก้ไขที่นี่
  
  if (bat_percentage >= 100) {
    bat_percentage = 100;
  }
  if (bat_percentage <= 0) {
    bat_percentage = 0; // เปลี่ยนเป็น 0 แทน 1
  }
  
  Serial.print("Analog Value = ");
  Serial.print(sensorValue);
  Serial.print("\t Output Voltage = ");
  Serial.print(voltage);
  Serial.print("\t Battery Percentage = ");
  Serial.println(bat_percentage);

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
  delay(1000);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
