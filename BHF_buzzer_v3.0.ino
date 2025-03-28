//#include <Arduino.h>

// BHF_buzzer_v3.0
// Final 21.03.2025

// Guckst Du https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#hardware-overview

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

//#define LED_PIN 15  // GPIO17 (D7) OnBoard-LED
#define LED_PIN D5
#define BAT_PIN D2
#define BUTTON_PIN D1
#define BUTTON_PIN_BITMASK (1ULL << GPIO_NUM_1) // GPIO 0 bitmask for ext1
#define FactorSeconds 1000000ULL
#define WIFI_FAIL_TIMEOUT 15000 // 10 sec wait time to enter sleep
#define VOLTAGE_FACTOR 0.002 // value to multiply the milivolts measured

// WLAN-Zugangsdaten
const char* WIFI_SSID =     "XXXX";
const char* WIFI_PASSWORD = "XXXX";

String Buzzer         = "GF1"; // Der Buzzer ist für diesen Raum
String slack_username = Buzzer + " Buzzer";
String slack_icon;
float voltage = 0;

// Slack Webhook URL (von Slack generiert, inklusive https://)
const char* slackWebhookUrl    = "https://hooks.slack.com/services/XXX";
const char* slackWebhookUrlBat = "https://hooks.slack.com/services/XXX";

// Funktion, die zufällig ein Slack-Emoji aus einer Liste auswählt
String getRandomSlackIcon() {
  // Liste von 20 Slack-Emojis
  String slack_icons[] = {
    ":smile:", ":fire:", ":thumbsup:", ":heart:", ":star:",
    ":sunglasses:", ":tada:", ":rocket:", ":zap:", ":sparkles:",
    ":clap:", ":grin:", ":wink:", ":joy:", ":ok_hand:",
    ":thinking_face:", ":smirk:", ":scream:", ":unamused:", ":flushed:"
  };

  int numIcons = sizeof(slack_icons) / sizeof(slack_icons[0]);
  int index = random(numIcons);  // Zufälliger Index zwischen 0 und numIcons-1
  return slack_icons[index];
}

// Nachrichten-Pool für Zufallsauswahl
String messages[] = {
    "Der {Buzzer} Buzzer wurde gedrückt.",
    "Harter Druck auf den Service-Knopf im {Buzzer}.",
    "Sanfter Buzzer-Druck im {Buzzer}.",
    "Schnell! Schnell! Der {Buzzer} ist fertig.",
    "Der {Buzzer} möchte bitte von dem Geschirrchaos bereinigt werden.",
    "Room Service bitte in den {Buzzer}.",
    "Alle feddisch im {Buzzer}. Frisches Wasser bitte.",
    "Diagnose im Raum {Buzzer}: Akute Meetingitis. Behandlung: gründliche Reinigung!",
    "Der Raum {Buzzer} hat erfolgreich ein Meeting überlebt. Jetzt braucht er dringend ein Desinfektionsteam!",
    "Housekeeping, bitte mit Notfallwagen zu Raum {Buzzer}. Die Flipcharts haben es nicht überlebt.",
    "Raum {Buzzer}: Patientenstatus stabil, aber der Besprechungstisch braucht dringend eine Behandlung!",
    "Kaffee-Katastrophencode in Raum {Buzzer}: Bitte mit Schwämmen und guter Laune ausrücken!",
    "Meeting abgeschlossen im Raum {Buzzer}. Symptome: Chaos und verlassene Kaffeetassen.",
    "Achtung, Housekeeping! Der Raum {Buzzer} liegt jetzt offiziell im Meeting-Koma. Bitte einleiten: Reinigungskur!",
    "Raum {Buzzer}: Diagnose 'Meetingspuren überall'. Therapie: gründliches Putzen und einmal lüften.",
    "Der Raum {Buzzer} meldet: Meeting beendet, aber das Whiteboard schreit nach Rettung.",
    "Housekeeping-Einsatz in Raum {Buzzer}: Die letzte Konferenz hinterließ Spuren – Kaffeeflecken inklusive.",
    "Code Coffee in Raum {Buzzer}: Überschuss an leeren Tassen. Bitte entleeren!",
    "Meeting erfolgreich beendet. Raum {Buzzer} benötigt eine Reinigung – und einen motivierten Housekeeping-Helden!",
    "Das Protokoll im Raum {Buzzer} ist abgeschlossen. Zeit für eure Intervention, Housekeeping!",
    "Notfall in Raum {Buzzer}: Der Flipchart-Marker wurde missbraucht. Reinigung dringend benötigt!",
    "Der Raum {Buzzer} ist bereit für die nächste Runde – aber erst nach eurer heldenhaften Reinigung!",
    "Krankheitsbild im Raum {Buzzer}: Post-Meeting-Syndrom. Therapie: Housekeeping mit Humor!",
    "Raum {Buzzer} meldet: Meeting abgeschlossen, Chaos hinterlassen. Bitte beseitigen!",
    "Bitte mit Lappen und Charme in Raum {Buzzer}: Die Kaffeekultur hinterlässt Spuren!",
    "Raum {Buzzer} an Housekeeping: Alle abgehauen, aber die Kekskrümel blieben zurück.",
    "Der Besprechungsraum {Buzzer} ist frei. Bitte macht ihn wieder fit für die nächste Gedankenschlacht!",
    "Sind alle abgehauen im {Buzzer} und haben alles stehen lassen",
    "Meeting vorbei, Kaffeeflecken inklusive! Der Raum {Buzzer} wartet auf ein bisschen Liebe (und einen Lappen).",
    "Breaking News: Der {Buzzer} ist bereit für die nächste Sitzung – sofern er die Reinigung überlebt.",
    "Raum {Buzzer}: Das Protokoll ist fertig, die Kekskrümel leider auch. Bitte übernehmen Sie!",
    "Der letzte Überlebende im Raum {Buzzer} hat mutig den Buzzer gedrückt. Jetzt seid ihr dran, Housekeeping!",
    "Der Raum {Buzzer} ist wie ein Tatort: Spuren sichern, aufräumen, und dann für den nächsten Fall freigeben!",
    "Mission erfüllt im Raum {Buzzer}. Zeit, die Schlacht um Flipchart, Kaffee und Chaos zu beenden!",
    "Achtung, Housekeeping: Der Raum {Buzzer} ist bereit, aber der Kekskrümel-Tsunami bleibt euer Problem.",
    "Im Raum {Buzzer} wurde der letzte Punkt abgearbeitet – jetzt wartet er auf euch, um den Rest zu beseitigen!",
    "Der Raum {Buzzer} meldet: Das Meeting ist vorbei. Jetzt kann der Wiederaufbau beginnen!",
    "Housekeeping Hero Mode aktiviert: Der Raum {Buzzer} braucht euch, bevor die nächste Konferenzlawine eintrifft!"
};

const size_t messageCount = sizeof(messages) / sizeof(messages[0]);

// ✅ **Funktion zur Auswahl & Ersetzung von `{Buzzer}` durch den tatsächlichen Namen**
String getRandomSlackMessage() {
    int index = random(0, messageCount);  // Zufällige Nachricht wählen
    String msg = messages[index];         // Nachricht aus Array holen
    Serial.println("# --- 🚀 Vor der Ersetzung: " + msg);  // Debug: Originalnachricht anzeigen
    msg.replace("{Buzzer}", Buzzer);      // Ersetzt "{Buzzer}" durch den aktuellen Raum-Namen
    Serial.println("# --- ✅ Nach der Ersetzung: " + msg);  // Debug: Ersetzte Nachricht anzeigen
    return msg;
}

// Funktion zum Senden der Nachricht an Slack
bool postMessageToSlack(String msg) {
    Serial.print("# --- Sende Nachricht an Slack: ");
    Serial.println(msg);

    WiFiClientSecure client;
    client.setInsecure();  // Falls kein Zertifikat genutzt wird

    HTTPClient http;
    http.begin(client, slackWebhookUrl);
    http.addHeader("Content-Type", "application/json");

    // Zufallszahlengenerator initialisieren (möglichst mit einem "zufälligen" Wert)
    randomSeed(analogRead(0));
    // Zufälliges Slack-Icon auswählen
    String slack_icon = getRandomSlackIcon();
    Serial.print("# --- Ausgewähltes Emoji: ");
    Serial.println(slack_icon);

    String payload = "{\"icon_emoji\": \"" + slack_icon + "\", \"username\": \"" + slack_username + "\", \"text\":\"" + msg + "\"}";
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        Serial.print("# --- Slack-Webhook Antwort: ");
        Serial.println(httpResponseCode);
        http.end();
        return (httpResponseCode == 200);  // Erfolg nur, wenn 200 OK
    } else {
        Serial.print("# --- Fehler beim Senden: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
    return true;
}

// Funktion zum Senden des Batteriestandes
bool postBatToSlack(String Batteriestand) {
    Serial.print("# --- Sende Batteriestand an Slack: ");
    Serial.println(Batteriestand);

    WiFiClientSecure client;
    client.setInsecure();  // Falls kein Zertifikat genutzt wird

    HTTPClient http;
    http.begin(client, slackWebhookUrlBat);
    http.addHeader("Content-Type", "application/json");

    // Setze das Slack Icon in abhängigkeit der Batteriespannung
    if (voltage < 3.00) {
      slack_icon = ":low_battery:";
    } else {
      slack_icon = ":battery:";
    }
    Serial.print("# --- Ausgewähltes Emoji: ");
    Serial.println(slack_icon);

    String payload = "{\"icon_emoji\": \"" + slack_icon + "\", \"username\": \"" + slack_username + "\", \"text\":\"" + Batteriestand + "\"}";
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        Serial.print("# --- Slack-Webhook Antwort: ");
        Serial.println(httpResponseCode);
        http.end();
        return (httpResponseCode == 200);  // Erfolg nur, wenn 200 OK
    } else {
        Serial.print("# --- Fehler beim Senden: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
    return true;
}

void goToDeepSleep(int seconds = 0) {
  if(seconds) {
    esp_sleep_enable_timer_wakeup(FactorSeconds * seconds);
  }
  digitalWrite(LED_PIN, LOW); // Licht aus
  //esp_deep_sleep_enable_gpio_wakeup(BUTTON_PIN_BITMASK, ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_LOW);
  esp_deep_sleep_disable_rom_logging();
  Serial.println("# --- Goto DEEP-Sleep - End of Code");
  Serial.println("# --- Goto DEEP-Sleep - End of Code");
  Serial.println("# --- Goto DEEP-Sleep - End of Code");
  esp_deep_sleep_start();
}

void blinkFail(int interval = 100) {
  // dont waste energy
  Serial.printf("# --- blink %d\r\n", interval);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  for(int i = 0; i < 5000/interval; ++i) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(interval);
  }
  goToDeepSleep();
}


void setup() {
    //delay(4000);
    Serial.begin(115200);
    //while(!Serial) {
    //  delay(10);
    //}
    Serial.println("# --- init setup");
    pinMode(LED_PIN, OUTPUT);  // LED-Pin als Ausgang setzen

    // ✅ OnBoard-LED direkt nach dem Start anschalten
    digitalWrite(LED_PIN, HIGH);  // LED AN

    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(100);
    digitalWrite(LED_PIN, LOW);

    // Reading Battery Voltage: https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#reading-battery-voltage
    voltage = VOLTAGE_FACTOR * analogReadMilliVolts(BAT_PIN);

    Serial.println("# --- init done");
}

bool slack_sent = false;
void loop() {
  uint32_t ti = millis();

  if (!slack_sent && (WiFi.status() == WL_CONNECTED)) {
    Serial.println("# --- ✅ WLAN verbunden!");
    Serial.print("# --- 📡 IP-Adresse: ");
    Serial.println(WiFi.localIP());
    Serial.printf("# --- Voltage %.2f\r\n", voltage);
    
    // Alter Code
    // WiFi.setAutoReconnect(true);
    // WiFi.persistent(false);

    // Slack-Nachrichten generieren
    String slackMessage  = getRandomSlackMessage();
    String Batteriestand = String("Die Batteriespannung vom " + slack_username + " liegt bei ") + String(voltage) + String(" Volt.");

    // ✅ Nachricht an Slack senden
    if(!postMessageToSlack(slackMessage)) {
      Serial.printf("# --- Slack post FAILED\r\n");
      blinkFail();
    } else {
      // Batteriestand senden
      postBatToSlack(Batteriestand);
      
      // dont waste energy
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      digitalWrite(LED_PIN, HIGH); // LED positive Bestätigung AN
      delay(5000);
      digitalWrite(LED_PIN, LOW); // LED aus

      Serial.println("# --- Goto PRE-Sleep, Serielle Schnittstelle bleibt an für ReCoding");
      // CPU-Frequenz reduzieren
      setCpuFrequencyMhz(80);         // z.B. auf 80MHz statt 240MHz
      Serial.println("# --- CPU jetzt bei 80 MHz für 2 Minuten mit aktivem Serial Output");
      // 2,0 Minuten warten (120000 Millisekunden)
      // 1,5 Minuten warten ( 90000 Millisekunden)
      // 1,0 Minuten warten ( 60000 Millisekunden)
      unsigned long start = millis();
      while (millis() - start < 120000) {
        // Durch kurze delay()-Intervalle wird der MCU weniger belastet.
        delay(100);
      }
      // Jetzt in den Deep Sleep Modus wechseln
      goToDeepSleep();
    }
    slack_sent = true;
  }

  if(millis() > WIFI_FAIL_TIMEOUT) {
    Serial.println("# --- WiFi fail");
    blinkFail(50);
  }
}
