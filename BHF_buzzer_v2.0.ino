
// Guckst Du https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#hardware-overview
//
// String Buzzer         = "XXX_RAUM_XXX"; // Der Buzzer ist für diesen Raum
// String slack_icon_url = ":fire:"; // War mal in 2020 funktional. Heute eher nicht mehr nötig
// String slack_username = "slackbuzzer";
// BHF und CKH - hauskeeping
// String slack_hook_url = "https://hooks.slack.com/services/XXX/XXX/XXX";
// Happenworld - in_der_burg
// String slack_hook_url = "https://hooks.slack.com/services/XXX/XXX/XXX";

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#define LED_PIN 15  // GPIO17 (D7)

// WLAN-Zugangsdaten
const char* WIFI_SSID     = "XXX";
const char* WIFI_PASSWORD = "XXX";

String Buzzer         = "V.505"; // Der Buzzer ist für diesen Raum
String slack_username = "V.505 Buzzer";
String slack_icon;

// Slack Webhook URL (von Slack generiert, inklusive https://)
const char* slackWebhookUrl = "https://hooks.slack.com/services/XXX/XXX/XXX";

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
    "Der {Buzzer} Buzzer wurde gedrückt",
    "Harter Druck auf den Service-Knopf im {Buzzer}",
    "Sanfter Buzzer-Druck im {Buzzer}",
    "Schnell! Schnell! Der {Buzzer} ist fertig",
    "Der {Buzzer} möchte bitte von dem Geschirrchaos bereinigt werden",
    "Room Service bitte in den {Buzzer}",
    "Alle feddisch im {Buzzer}",
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

// Sendeintervall (ms)
const unsigned long sendInterval = 60000;  // 60 Sekunden

unsigned long lastSendTime = 0;


// ✅ **Funktion zur Auswahl & Ersetzung von `{Buzzer}` durch den tatsächlichen Namen**
String getRandomSlackMessage() {
    int index = random(0, messageCount);  // Zufällige Nachricht wählen
    String msg = messages[index];         // Nachricht aus Array holen
    Serial.println("🚀 Vor der Ersetzung: " + msg);  // Debug: Originalnachricht anzeigen
    msg.replace("{Buzzer}", Buzzer);      // Ersetzt "{Buzzer}" durch den aktuellen Raum-Namen
    Serial.println("✅ Nach der Ersetzung: " + msg);  // Debug: Ersetzte Nachricht anzeigen
    return msg;
}

// Funktion zum Senden der Nachricht an Slack
bool postMessageToSlack(String msg) {
    Serial.print("Sende Nachricht an Slack: ");
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
    Serial.print("DEBUG: Ausgewähltes Emoji: ");
    Serial.println(slack_icon);

    String payload = "{\"icon_emoji\": \"" + slack_icon + "\", \"username\": \"" + slack_username + "\", \"text\":\"" + msg + "\"}";
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        Serial.print("Slack-Webhook Antwort: ");
        Serial.println(httpResponseCode);
        http.end();
        return (httpResponseCode == 200);  // Erfolg nur, wenn 200 OK
    } else {
        Serial.print("Fehler beim Senden: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
}

// ✅ **ESP32 in goToLowPowerMode versetzen**
void goToDeepSleep() {
    Serial.println("🔋 ESP geht in stromsparenden Modus...");
    delay(100);  // Kurz warten, um sicherzustellen, dass Logs gesendet werden
    // ✅ Endlosschleife → ESP macht "nichts" mehr, bleibt aber per USB ansprechbar
    while (1) {
        delay(100000);  
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);  // LED-Pin als Ausgang setzen
    
    // ✅ OnBoard-LED direkt nach dem Start ausschalten
    digitalWrite(LED_PIN, HIGH);  // LED AUS

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("🌍 Verbinde mit WLAN...");
        unsigned long startTime = millis();  // Startzeit für WiFi-Timeout

        // Versuche 5 Sekunden lang, WiFi zu verbinden
        while ((WiFi.status() != WL_CONNECTED) && (millis() - startTime < 5000)) {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            break;  // Falls WiFi erfolgreich verbunden ist, beende die Schleife
        }

        Serial.println("\n⚠️ WLAN-Verbindung fehlgeschlagen. Erneuter Versuch...");

        // LED für 2 Sekunden im 50ms-Takt blinken lassen
        for (int i = 0; i < 40; i++) {  // 2 Sekunden / 50ms = 40 Blinkzyklen
            digitalWrite(LED_PIN, HIGH);
            delay(50);
            digitalWrite(LED_PIN, LOW);
            delay(50);
        }
        digitalWrite(LED_PIN, HIGH); // LED erst mal wieder ausschalten
    }
    Serial.println("\n✅ WLAN verbunden!");
    Serial.print("📡 IP-Adresse: ");
    Serial.println(WiFi.localIP());

    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);

    // Slack-Nachricht generieren
    String slackMessage = getRandomSlackMessage();

    // ✅ Nachricht an Slack senden
    if (postMessageToSlack(slackMessage)) {
        Serial.println("✔️ Nachricht erfolgreich an Slack gesendet.");
        // LED für 10 Sekunden einschalten
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, LOW);
        delay(10000);  // 10 Sekunden warten
        digitalWrite(LED_PIN, HIGH);  // LED ausschalten
        goToDeepSleep();  // 🔴 Nach erfolgreichem Senden ESP in DeepSleep versetzen
    } else {
        Serial.println("❌ Fehler beim Senden der Nachricht.");
        // LED für 10 Sekunden blinken (200ms Takt)
        pinMode(LED_PIN, OUTPUT);
        for (int i = 0; i < 50; i++) {  // 50 Mal blinken (10 Sek. / 200ms = 50)
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
        goToDeepSleep();  // 🔴 Selbst wenn Slack fehlschlägt, ESP in DeepSleep versetzen
    }
}

void loop() {
    // **Loop ist leer, weil der ESP nach setup() in DeepSleep geht**
}
