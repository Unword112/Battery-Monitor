<?php
$input = file_get_contents("php://input");
error_log("Received data: " . $input);  // บันทึกข้อมูลที่ได้รับจาก Arduino หรือ LINE

$data = json_decode($input, true);

// ตรวจสอบว่า LINE ส่งข้อความ "Battery status" หรือไม่
session_start();
if (isset($data['battery_percentage'])) {
    $_SESSION['battery_percentage'] = $data['battery_percentage'];
}

// เมื่อได้รับข้อความ "Battery status" จาก LINE
if (isset($data['events'])) {
    foreach ($data['events'] as $event) {
        if ($event['type'] == 'message' && $event['message']['text'] == 'Battery status') {
            if (isset($_SESSION['battery_percentage'])) {
                $batteryLevel = $_SESSION['battery_percentage'];
                $replyToken = $event['replyToken'];

                // ส่งข้อความไปยัง LINE
                $message = 'Battery Level: ' . $batteryLevel . '%';

                // ส่งคำตอบกลับไปยัง LINE
                $headers = [
                    'Content-Type: application/json',
                    'Authorization: Bearer YOUR_CHANNEL_ACCESS_TOKEN'
                ];

                $body = json_encode([
                    'replyToken' => $replyToken,
                    'messages' => [['type' => 'text', 'text' => $message]]
                ]);

                $ch = curl_init('https://api.line.me/v2/bot/message/reply');
                curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
                curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
                curl_setopt($ch, CURLOPT_POST, true);
                curl_setopt($ch, CURLOPT_POSTFIELDS, $body);
                $response = curl_exec($ch);
                curl_close($ch);

                if ($response === false) {
                    error_log("Error sending response to LINE: " . curl_error($ch));
                } else {
                    error_log("Message sent successfully: " . $response);
                }
            } else {
                error_log("No battery percentage available.");
                echo json_encode(["status" => "error", "message" => "No battery percentage available."]);
            }
        }
    }
}

echo 'OK';
?>
