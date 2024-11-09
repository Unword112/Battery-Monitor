#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>

// ข้อมูล WiFi
const char* ssid = "pok";
const char* password = "12345678n";

// ข้อมูล ThingsBoard
String tbHost = "thingsboard.cloud";  
String tbToken = "n5ksm170z9xuwo65mz45"; 

// ข้อมูล LINE Messaging API
String lineToken = "J8C//G7Lmm4gWLytK4f6dPsweAnI24NYSCslov+Qv49XVXuzzb+ZBu31P+sJy/83Zke1uEUK0Pxb4ZwKnsUC63+rOF0VOZTmOuAe76jv38UTy7gLAETTeBO+KHn+kPVC9I22hbp44rQKtvOJEZKxvgdB04t89/1O/w1cDnyilFU=";
String phpServer = "https://4a84-1-46-74-211.ngrok-free.app/line_webhook/webhook.php";  // URL ของ PHP Server
WebServer server(8080);

// ข้อมูล Battery
int analogInPin  = 34;
int sensorValue; 
float voltage;
float bat_percentage;

void sendBatteryDataToPHP(float batteryPercentage) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // กำหนด URL ของ PHP server
    http.begin(phpServer);
    http.addHeader("Content-Type", "application/json");
    
    // สร้าง JSON ข้อมูลที่ต้องการส่ง
    String jsonData = "{\"sensorValue\": " + String(sensorValue) + 
                        ", \"voltage\": " + String(voltage) +
                        ", \"battery_percentage\": " + String(batteryPercentage) + "}";
    
    // ส่งข้อมูลไปยัง PHP server
    int httpResponseCode = http.POST(jsonData);
    
    if (httpResponseCode > 0) {
      Serial.println("Data sent to PHP server successfully.");
    } else {
      Serial.print("Error sending data: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}

// ฟังก์ชันการส่งข้อความไปยัง LINE
void sendLineMessage(String replyToken, String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverName = "https://api.line.me/v2/bot/message/reply";
    
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + lineToken);

    String jsonData = "{\"replyToken\":\"" + replyToken + "\"," 
                      "\"messages\":[{\"type\":\"text\",\"text\":\"" + message + "\"}]}";
    
    Serial.println("Sending reply: " + jsonData);  // เพิ่มการพิมพ์ JSON ที่จะส่งไป

    int httpResponseCode = http.POST(jsonData);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      Serial.println("Message sent successfully.");
    } else {
      Serial.println("Failed to send message.");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}

void handleWebhook() {
  Serial.println("Received POST request");
  
  if (server.method() == HTTP_POST) {
    String body = server.arg("plain");
    Serial.println("Body received: " + body);
    
    // ตรวจสอบว่าข้อมูลที่ได้รับจาก LINE มีข้อความหรือไม่
    if (body.indexOf("\"text\":\"Battery status\"") != -1) {
      String replyToken;
      int startIdx = body.indexOf("\"replyToken\":\"") + 14;
      int endIdx = body.indexOf("\"", startIdx);
      if (startIdx != -1 && endIdx != -1) {
        replyToken = body.substring(startIdx, endIdx);
        sendLineMessage(replyToken, "Battery Level: " + String(bat_percentage) + "%");
      }
    }

    if (body.indexOf("\"text\":\"Battery Voltage\"") != -1) {
      String replyToken;
      int startIdx = body.indexOf("\"replyToken\":\"") + 14;
      int endIdx = body.indexOf("\"", startIdx);
      if (startIdx != -1 && endIdx != -1) {
        replyToken = body.substring(startIdx, endIdx);
        sendLineMessage(replyToken, "Voltage: " + String(voltage) + "V");
      }
    }

    if (body.indexOf("\"text\":\"Sensor Value\"") != -1) {
      String replyToken;
      int startIdx = body.indexOf("\"replyToken\":\"") + 14;
      int endIdx = body.indexOf("\"", startIdx);
      if (startIdx != -1 && endIdx != -1) {
        replyToken = body.substring(startIdx, endIdx);
        sendLineMessage(replyToken, "Analog Value: " + String(sensorValue));
      }
    }
    
    // ส่ง response กลับไปยัง LINE
    server.send(200, "application/json", "{\"status\":\"received\"}");
  } else {
    Serial.println("Not a POST request");
    server.send(405, "application/json", "{\"status\":\"method not allowed\"}");
  }
}

void handleBatteryRequest() {
  String batteryData = "{\"battery_percentage\": " + String(bat_percentage) + "}";
  server.send(200, "application/json", batteryData);
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  server.on("/get_battery", HTTP_GET, handleBatteryRequest);
  server.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.on("/webhook", HTTP_POST, handleWebhook);
  server.begin();
  Serial.println("HTTP server started on port 80");
}

void loop() {
  server.handleClient();
  sendBatteryDataToPHP(bat_percentage);

  sensorValue = analogRead(analogInPin);
  voltage = (sensorValue * 3.3) / 4095.0;
  bat_percentage = (voltage / 4.2) * 100;

  // Ensure battery percentage is within the 0-100% range
  bat_percentage = constrain(bat_percentage, 0, 100);
  
  Serial.print("Analog Value = ");
  Serial.print(sensorValue);
  Serial.print("\t Output Voltage = ");
  Serial.print(voltage);
  Serial.print("\t Battery Percentage = ");
  Serial.println(bat_percentage);

  String serverName = "http://" + tbHost + "/api/v1/" + tbToken + "/telemetry";

  // Send battery data to ThingsBoard
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"percentage\": " + String(bat_percentage) +
                      ", \"Analog_Value\": " + String(sensorValue) +
                      ", \"Output_Voltage\": " + String(voltage) + "}";

    int httpResponseCode = http.POST(jsonData);
    if (httpResponseCode > 0) {
      Serial.println("Data sent to ThingsBoard!");
    } else {
      Serial.print("Error sending data: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  if (WiFi.status() != WL_CONNECTED) {
  server.send(503, "application/json", "{\"status\":\"WiFi not connected\"}");
  return;
}

  // Send low battery notification to LINE when percentage is below 20%
  if (bat_percentage < 20) {
    String lineMessage = "Battery Low: " + String(bat_percentage) + "%";
    sendLineMessage("", lineMessage); // Send to LINE
  }

  delay(60000); 
}