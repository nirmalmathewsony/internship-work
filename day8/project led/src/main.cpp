#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// --- Configuration ---
const char* ssid = "iQOO110";
const char* password = "nirmal10";

// GPIO pin where the LED is connected
const int ledPin = 2; // GPIO 2 is usually the built-in blue LED on most ESP32 boards
bool ledState = LOW;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// --- HTML / Website Code ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 LED Control</title>
    <style>
        html { font-family: Arial, Helvetica, sans-serif; text-align: center; background-color: #f4f4f9;}
        h2 { color: #333; margin-top: 50px; }
        .button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;
                  text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; border-radius: 8px;}
        .button2 {background-color: #555555;}
    </style>
</head>
<body>
    <h2>ESP32 Local Web Server</h2>
    <p>Click below to toggle the LED state.</p>
    <p><a href="/toggle"><button class="button">TOGGLE LED</button></a></p>
</body>
</html>
)rawliteral";

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    
    // Initialize LED pin as an output
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, ledState);

    // Connect to Wi-Fi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    // Print ESP32 Local IP Address
    Serial.println("");
    Serial.println("Wi-Fi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // --- Web Routes ---
    
    // Route for Root / Web Page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    // Route to handle the toggle button action
    server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
        ledState = !ledState;
        digitalWrite(ledPin, ledState);
        // Redirect back to the main page after changing the state
        request->redirect("/");
    });

    // Start server
    server.begin();
}

void loop() {
    // ESPAsyncWebServer runs asynchronously in the background. 
    // No code is needed in the loop for handling web requests!
}