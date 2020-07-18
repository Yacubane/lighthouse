#include "LightClient.h"

void HClient::setClient(WiFiClient client) {
    this->wifiClient = client;
    this->lastKeepalive = millis();
    this->active = true;
    this->authenticated = false;
    this->subscribedToEverything = false;
    this->error = false;
}

void HClient::disconnect() {
    this->wifiClient.stop();
    this->active = false;
    this->authenticated = false;
    this->subscribedToEverything = false;
    this->error = false;
}

String HClient::setError() {
    this->error = true;
    return "";
}

int HClient::timedRead() {
    int c;
    unsigned long _startMillis = millis();
    do {
        c = this->wifiClient.read();
        if(c >= 0)
            return c;
        yield();
    } while(millis() - _startMillis < READ_TIMEOUT);
    return -1;     // -1 indicates timeout
}

String HClient::receiveJsonObjectString() {
    if (!this->wifiClient.available()) {
        return setError();
    }

    int first = this->timedRead();
    while (first != -1 && (char)first != '{') {
        first = this->timedRead();
    }

    if (first == -1) {
        return String("");
    }

    String jsonText = "{";
    int bracketOpenings = 1;
    while (bracketOpenings > 0) {
        int character = this->timedRead();
        if (character == -1) {
            return setError();
        }
        if ((char)character == '{') {
            bracketOpenings++;
        }
        if ((char)character == '}') {
            bracketOpenings--;
        }
        if ((char)character == ' ') {
            continue;
        }
        jsonText += (char) character;
    }

    return jsonText;
}

DynamicJsonDocument HClient::receiveJsonObject() {
    String text = receiveJsonObjectString();
    DynamicJsonDocument doc(JSON_CAPACITY);
    if (!text.equals("")) {
        deserializeJson(doc, text);
    }
    return doc;
}