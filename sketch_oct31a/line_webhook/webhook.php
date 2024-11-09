<?php
$input = file_get_contents("php://input");
error_log("Received data: " . $input);  // บันทึกข้อมูลที่ได้รับจาก Arduino หรือ LINE

$data = json_decode($input, true);
$batteryFile = 'battery_status.json';

$existingData = file_exists($batteryFile) ? json_decode(file_get_contents($batteryFile), true) : [];


// ตรวจสอบว่ามีข้อมูลแบตเตอรี่ที่ส่งมาจาก ESP32
if (isset($data['battery_percentage'])) {
    $existingData['battery_percentage'] = $data['battery_percentage'];
    error_log("Battery percentage saved: " . $data['battery_percentage']);
}

if (isset($data['sensorValue'])) {
    $existingData['sensorValue'] = $data['sensorValue'];
    error_log("Sensor value saved: " . $data['sensorValue']);
}

if (isset($data['voltage'])) {
    $existingData['voltage'] = $data['voltage'];
    error_log("Voltage saved: " . $data['voltage']);
}

file_put_contents($batteryFile, json_encode($existingData, JSON_PRETTY_PRINT));
echo json_encode(["status" => "success"]);

// เมื่อได้รับข้อความ "Battery status" จาก LINE
if (isset($data['events'])) {
    foreach ($data['events'] as $event) {
        if ($event['type'] == 'message') {
            $replyToken = $event['replyToken'];
            $messageText = strtolower($event['message']['text']);

            // อ่านข้อมูลจากไฟล์ JSON
            $batteryData = file_exists($batteryFile) ? json_decode(file_get_contents($batteryFile), true) : null;

            $replyMessage = "";
            if ($batteryData) {
                if ($messageText == 'battery status') {
                    $replyMessage = "Battery Level: " . $batteryData['battery_percentage'] . "%";
                } elseif ($messageText == 'battery voltage') {
                    $replyMessage = "Voltage: " . $batteryData['voltage'] . "V";
                } elseif ($messageText == 'sensor value') {
                    $replyMessage = "Analog Value: " . $batteryData['sensorValue'];
                } else {
                    $replyMessage = "Unknown command.";
                }
            } else {
                $replyMessage = "Battery data not available.";
            }

            // ส่งข้อความไปยัง LINE Bot
            $headers = [
                'Content-Type: application/json',
                'Authorization: Bearer LINE_ACESS_TOKEN'
            ];

            $body = json_encode([
                'replyToken' => $replyToken,
                'messages' => [['type' => 'text', 'text' => $replyMessage]]
            ]);

            $ch = curl_init('https://api.line.me/v2/bot/message/reply');
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
            curl_setopt($ch, CURLOPT_POST, true);
            curl_setopt($ch, CURLOPT_POSTFIELDS, $body);
            $response = curl_exec($ch);
            
            if (curl_errno($ch)) {
                error_log("Error sending response to LINE: " . curl_error($ch));
            } else {
                error_log("Message sent successfully: " . $response);
            }
            curl_close($ch);
        }
    }
}

echo 'OK';
?>
