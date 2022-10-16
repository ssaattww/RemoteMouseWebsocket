#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <M5Core2.h>
#include <map>
#include "defines.h"
#include "BleMouse.h"

static WebSocketsServer webSocket(81);
IPAddress ipadr;
DynamicJsonDocument doc(1024);
BleMouse bleMouse;

/// @brief parse json payload
/// @param payload json string
/// @return parsed array
JsonArray parseReceivedJson(uint8_t *payload)
{
 	char *json = (char *)payload;
 	DeserializationError error = deserializeJson(doc, json);
 
	if (error) {
	Serial.print(F("deserializeJson() failed: "));
	Serial.println(error.c_str());
	return JsonArray();
	}

 	JsonObject obj = doc.as<JsonObject>();
	return obj["command_list"].as<JsonArray>();
}

/// @brief websocket recieved event
/// @param num client number
/// @param type event type
/// @param payload recieved payload
/// @param length payload size
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
	switch(type) {
	case WStype_DISCONNECTED:
		// 切断時の処理;
		{
			IPAddress ip = webSocket.remoteIP(num); // クライアントのIPアドレスを取得
			M5.Lcd.setCursor(0,20);
			M5.Lcd.print("disconnect client      \n");
			M5.Lcd.print(ip.toString());
			M5.Lcd.print("      ");
		}
		break;
	case WStype_CONNECTED:
		{
			IPAddress ip = webSocket.remoteIP(num); // クライアントのIPアドレスを取得
			M5.Lcd.setCursor(0,20);
			M5.Lcd.print("connect client        \n");
			M5.Lcd.print(ip.toString());
			M5.Lcd.print("      ");
			// 接続時の処理
		}
		break;
	case WStype_TEXT:
		// テキストデータ受信時の処理;
		{
			JsonArray arr = parseReceivedJson(payload);
			M5.Lcd.setCursor(0,50);
			for(JsonVariant v : arr) {
				for(JsonVariant v2 : v.as<JsonArray>())M5.Lcd.printf("%d,",v2.as<uint8_t>());
				if(bleMouse.isConnected()){
					bleMouse.buttons(v[0].as<uint8_t>());
					bleMouse.move(v[1].as<char>(),v[2].as<char>(),v[3].as<char>(),v[4].as<char>());
				}
				M5.Lcd.print("\n");
			}
		}
		break;
	case WStype_BIN:
		// バイナリデータ受信時の処理;
		break;
	case WStype_ERROR:
		// エラー時の処理;
		break;
	}
}

/// @brief setup wifi configrations
void setupWiFi()
{
	WiFi.mode(WIFI_STA);
 	WiFi.begin(SSID, PASS);

	// Wait some time to connect to wifi
	for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
		Serial.print(".");
		delay(1000);
 	}

 	// Check if connected to wifi
	if(WiFi.status() != WL_CONNECTED) {
		Serial.println("No Wifi!");
		return;
	}

	ipadr = WiFi.localIP();
 	Serial.println("Connected to Wifi");
	
	M5.Lcd.setCursor(0,0);
	M5.Lcd.print(ipadr.toString());
	// server address, port and URL
	webSocket.begin();
	// event handler
	webSocket.onEvent(webSocketEvent);
}

void setup()
{
	Serial.begin(115200);
	bleMouse.begin();
	// Power ON Stabilizing...
	delay(500);
	M5.begin();

	M5.Lcd.fillScreen(BLACK);
	M5.Lcd.setTextColor(GREEN,BLACK);

	M5.Lcd.setTextSize(2);
	setupWiFi();
}

void loop() {
	webSocket.loop();

	M5.update();
}