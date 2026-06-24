#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// --- Configuration ---
const char* ssid = "iQOO110";
const char* password = "nirmal10";

// LED Pin configuration (Most ESP32 boards have a built-in LED on GPIO 2)
const int LED_PIN = 2; 

// PWM Settings for ESP32 LEDC
const int PWM_FREQ = 5000;
const int PWM_CHANNEL = 0;
const int PWM_RESOLUTION = 8; // 8-bit resolution (0-255)

// State variables
int currentBrightness = 0;
bool isFading = false;
int fadeDirection = 1; // 1 for brightening, -1 for dimming
unsigned long lastFadeTime = 0;
const int fadeInterval = 15; // Speed of fading in milliseconds

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// --- HTML & CSS UI ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 LED Controller</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background-color: #121212; color: white; padding: 20px; }
        h2 { color: #00adb5; }
        .btn { display: inline-block; padding: 15px 30px; font-size: 18px; cursor: pointer; border: none; border-radius: 5px; color: white; margin: 10px; width: 150px; }
        .btn-on { background-color: #28a745; }
        .btn-off { background-color: #dc3545; }
        .btn-fade { background-color: #ffc107; color: black; }
        .slider-container { margin: 30px auto; max-width: 300px; }
        .slider { width: 100%; height: 15px; border-radius: 5px; background: #393e46; outline: none; opacity: 0.7; transition: opacity .2s; }
        .slider:hover { opacity: 1; }
    </style>
</head>
<body>
    <h2>ESP32 LED Control Panel</h2>
    
    <div>
        <button class="btn btn-on" onclick="sendRequest('/on')">Turn ON</button>
        <button class="btn btn-off" onclick="sendRequest('/off')">Turn OFF</button>
    </div>
    
    <div class="slider-container">
        <h3>Brightness: <span id="val">0</span></h3>
        <input type="range" min="0" max="255" value="0" class="slider" id="brightnessSlider" oninput="updateBrightness(this.value)">
    </div>

    <div>
        <button class="btn btn-fade" onclick="sendRequest('/fade')">Toggle Fade</button>
    </div>

    <script>
        function sendRequest(url) {
            fetch(url);
        }
        function updateBrightness(val) {
            document.getElementById("val").innerText = val;
            fetch('/brightness?value=' + val);
        }
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);

    // Set up PWM for ESP32
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(LED_PIN, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, currentBrightness);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected! IP Address: ");
    Serial.println(WiFi.localIP());

    // --- Web Server Routes ---
    
    // Serve Root HTML Page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    // Route to turn LED fully ON
    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
        isFading = false;
        currentBrightness = 255;
        ledcWrite(PWM_CHANNEL, currentBrightness);
        request->send(200, "text/plain", "OK");
    });

    // Route to turn LED OFF
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
        isFading = false;
        currentBrightness = 0;
        ledcWrite(PWM_CHANNEL, currentBrightness);
        request->send(200, "text/plain", "OK");
    });

    // Route to handle Brightness Slider
    server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("value")) {
            isFading = false;
            String val = request->getParam("value")->value();
            currentBrightness = val.toInt();
            ledcWrite(PWM_CHANNEL, currentBrightness);
        }
        request->send(200, "text/plain", "OK");
    });

    // Route to toggle Fading mode
    server.on("/fade", HTTP_GET, [](AsyncWebServerRequest *request){
        isFading = !isFading;
        request->send(200, "text/plain", "OK");
    });

    // Start Server
    server.begin();
}

void loop() {
    // Non-blocking fade logic handled in main loop
    if (isFading) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastFadeTime >= fadeInterval) {
            lastFadeTime = currentMillis;
            
            currentBrightness += fadeDirection;
            
            if (currentBrightness >= 255) {
                currentBrightness = 255;
                fadeDirection = -1; // Start dimming
            } else if (currentBrightness <= 0) {
                currentBrightness = 0;
                fadeDirection = 1;  // Start brightening
            }
            
            ledcWrite(PWM_CHANNEL, currentBrightness);
        }
    }
}