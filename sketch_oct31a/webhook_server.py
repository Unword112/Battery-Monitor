from flask import Flask, request, jsonify
import requests

app = Flask(__name__)

LINE_ACCESS_TOKEN = '{process.env.ACCESS_TOKEN}'
ESP32_IP = 'http://127.0.0.1'  # IP ของ ESP32 ในเครือข่ายของคุณ

def reply_message(reply_token, message):
    url = 'https://api.line.me/v2/bot/message/reply'
    headers = {
        'Content-Type': 'application/json',
        'Authorization': 'Bearer ' + LINE_ACCESS_TOKEN
    }
    data = {
        'replyToken': reply_token,
        'messages': [{'type': 'text', 'text': message}]
    }
    requests.post(url, headers=headers, json=data)

@app.route("/webhook", methods=['POST'])
def webhook():
    event = request.json['events'][0]
    reply_token = event['replyToken']
    user_message = event['message']['text']

    if user_message.lower() == "status":
        # ขอข้อมูลสถานะแบตเตอรี่จาก ESP32
        response = requests.get(ESP32_IP + "/get_status")
        if response.status_code == 200:
            battery_status = response.json()
            message = f"Battery: {battery_status['percentage']}%, Voltage: {battery_status['voltage']}V"
        else:
            message = "Error retrieving battery status."
        reply_message(reply_token, message)

    return jsonify({"status": "ok"})

if __name__ == "__main__":
    app.run(port=5000)
