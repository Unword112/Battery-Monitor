"# Battery-Monitor" 
"# Battery-Monitor" 

Using ESP32 to Monitor Battery 3.7V 4 Pieces

Use Arduino for programming ESP32. Send Data to Thingsboard and Chat Line Bot

Use ngrok for POST, GET the webhook
Example : https://4axx-xx-xx-xx-xxx.ngrok-free.app  -> http://localhost:8080  

Use Xampp for open port (like, for example, ngrok -> http://localhost:8080 
Apache 443, 8080 
Create a new file in folder C:\xampp\htdocs (in example, file name: line_webhook) 
Create file .php ((In example file name : webhook.php)

On Line Developer Console Setting for use webhook setting up the WEBHOOK URL
Example : https://4axx-xx-xx-xx-xxx.ngrok-free.app/line_webhook/webhook.php
And turn Use webhook -> ON

When ESP32 connects to WiFi, it will send data to php_error_log (C:\xampp\php\logs) #if it hasn't a file ( logs ) create it and check on ( php.ini ) 
Example in php_error_log : 09-Nov-2024 10:17:44 Europe/Berlin] Received data: {"sensorValue": 3643, "voltage": 2.94, "battery_percentage": 69.90}

Code on webhook.php will receive data and display on php_error_log : [09-Nov-2024 10:19:50 Europe/Berlin] Battery percentage saved: 69.82
                                                                     [09-Nov-2024 10:19:50 Europe/Berlin] Sensor value saved: 3639
                                                                     [09-Nov-2024 10:19:50 Europe/Berlin] Voltage saved: 2.93
And will create JSON file for store data : 
{
    "battery_percentage": 69.82,
    "sensorValue": 3639,
    "voltage": 2.93
}
When new data comes in, it will overwrite the old one.

Code on Arduino, in function sendLineMessage() will send data to php_error_log when you message in Line Bot
[09-Nov-2024 10:00:00 Europe/Berlin] Received data: String jsonData = "{\"replyToken\":\"" + replyToken + "\"," "\"messages\":[{\"type\":\"text\",\"text\":\"" + message + "\"}]}";
in function handleWebhook() wil send data when you message 3 things : Battery status, Battery Voltage, Sensor Value In the Line Bot, you will get 3 different data points.
