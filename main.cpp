#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define PIN_RELAY_1 5
#define WAIT_TIME_NEXT_REQUEST 60

const char* wifiSsid = "flicker";
const char* wifiPassword = "0246813579";

struct Message {
  unsigned long id;
  unsigned long delay;
  String content;
};

void setup() {
  pinMode(PIN_RELAY_1, OUTPUT);
  digitalWrite(PIN_RELAY_1, LOW);

  Serial.begin(9600);
  Serial.println();
  delay(3000);
  Serial.println("Connecting to WiFi.");

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println();
  Serial.println("WiFi connected.");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    String responseRequestApiMessageIndex;
    bool isSuccessSendRequestApiMessageIndex = sendRequestApiMessageIndex(responseRequestApiMessageIndex);

    if (isSuccessSendRequestApiMessageIndex) {
      Message message;
      bool isSuccessReadResponseApiMessageIndex = readResponseApiMessageIndex(responseRequestApiMessageIndex, message);

      if (isSuccessReadResponseApiMessageIndex) {
        flashSequence(message.delay, message.content);

        bool isSuccessSendRequestApiMessageUpdate = sendRequestApiMessageUpdate(message.id);

        if (isSuccessSendRequestApiMessageUpdate) {
          //
        }
      }
    }
  }

  delay(1000 * WAIT_TIME_NEXT_REQUEST);
}


bool sendRequestApiMessageIndex(String& response) {
  HTTPClient http;
  http.begin("http://flicker.azate.org/api/public/message");
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    response = http.getString();

    http.end();

    return response.length() > 2; // []
  }

  return false;
}

bool readResponseApiMessageIndex(String response, struct Message& message) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(response);

  if (data.success()) {
    message.id = data["id"];
    message.delay = data["delay"];
    message.content = data["content"].asString();

    return true;
  }

  return false;
}

bool sendRequestApiMessageUpdate(unsigned long id) {
  HTTPClient http;
  http.begin(String("http://flicker.azate.org/api/public/message/") + id + "/update");
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();

    http.end();

    return response.length() > 0 && response == "ok";
  }

  return false;
}

void flashSequence(unsigned long ms, String message) {
  for (int i = 0; i <= message.length(); i++) {
    switch (message[i]) {
      case '.':
        digitalWrite(PIN_RELAY_1, HIGH);
        delay(ms);
        digitalWrite(PIN_RELAY_1, LOW);
        delay(ms);
        break;

      case '-':
        digitalWrite(PIN_RELAY_1, HIGH);
        delay(ms * 3);
        digitalWrite(PIN_RELAY_1, LOW);
        delay(ms);
        break;

      case ' ':
        delay(ms * 2); // 3
        break;

      case '/':
        delay(ms * 3); // 7
        break;
    }
  }
}
