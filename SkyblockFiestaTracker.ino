#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* url = "https://skyblock.dev/election/";

unsigned long lastCheck = 0;
const unsigned long checkInterval = 3600000; // 1 hour (ms)

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastCheck >= checkInterval || lastCheck == 0) {
    lastCheck = currentMillis;
    checkMayorAndPerks();
  }

  delay(1000);
}

void checkMayorAndPerks() {
  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();

    // Find section after <h1>Current Mayor and Minister</h1>
    int h1Index = payload.indexOf("Current Mayor and Minister");
    if (h1Index == -1) {
      Serial.println("Couldn't find the main section.");
      return;
    }

    // Find first <h2> (Mayor)
    int h2Start = payload.indexOf("<h2", h1Index);
    int h2Close = payload.indexOf("</h2>", h2Start);
    String mayor = extractTagText(payload, h2Start, h2Close);

    // Gather following <p> tags until next <h2>
    int nextH2 = payload.indexOf("<h2", h2Close + 1);
    String perks = payload.substring(h2Close, nextH2);

    Serial.println("Mayor: " + mayor);
    Serial.println("Perks section:\n" + perks);

    bool fiestaFound = perks.indexOf("fiesta") != -1 || perks.indexOf("Fiesta") != -1;
    bool jerryFound = mayor.indexOf("Jerry") != -1;

    if (fiestaFound || jerryFound) {
      Serial.println("🎉 Fiesta or Jerry detected! Show image A");
      // TODO: display.showImage(imageA);
    } else {
      Serial.println("❌ No Fiesta or Jerry. Show image B");
      // TODO: display.showImage(imageB);
    }

  } else {
    Serial.printf("HTTP GET failed, code: %d\n", httpCode);
  }

  http.end();
}

String extractTagText(String &html, int tagStart, int tagClose) {
  int start = html.indexOf(">", tagStart);
  if (start == -1 || tagClose == -1) return "";
  return html.substring(start + 1, tagClose);
}
