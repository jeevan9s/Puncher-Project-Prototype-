#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

const int MPU = 0x68;
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
float total_accel;
const float punchThreshold = 7;
bool punchActive = false;
unsigned long punchStartTime = 0;
unsigned long punchEndTime = 0;
float totalAccelDuringPunch = 0;
int punchCount = 0;
int accelCount = 0;
float weight = 60.0;
float averageAccel = 0;

// Replace with your network credentials
const char* ssid = "dlinkCETA";
const char* password = "ceta2024";

WebServer server(80);

void convert_accel(float Xvalue, float Yvalue, float Zvalue) {
    Xvalue /= 1350;
    Xvalue -= 1;

    Zvalue /= 1850;
    Zvalue -= 9.86;

    Yvalue /= 1350;
    Yvalue -= 1;

    AcX = Xvalue;
    AcY = Yvalue;
    AcZ = Zvalue;
}

void acceleration_finder(float Xval, float Yval, float Zval) {
    int count = 0;
    if (Yval == 0) {
        Yval += 1;
        count += 1;
    }
    if (Xval == 0) {
        Xval += 1;
        count += 1;
    }
    if (count == 1) {
        total_accel = sqrt(pow(Xval, 2) + pow(Yval, 2) - 1);
    } else if (count == 2) {
        total_accel = sqrt(pow(Xval, 2) + pow(Yval, 2) - 2);
    } else {
        total_accel = sqrt(pow(Xval, 2) + pow(Yval, 2));
    }
}

void handleRoot() {
    String html = "<html><head><title>Punching Data</title></head><body><h1>Punching Data</h1>";
    html += "<p>Weight: " + String(weight) + " kg</p>";
    html += "<form id=\"weightForm\"><label for=\"weight\">Enter Weight (kg):</label>";
    html += "<input type=\"number\" id=\"weight\" name=\"weight\" value=\"" + String(weight) + "\">";
    html += "<button type=\"button\" onclick=\"submitWeight()\">Submit</button></form>";
    html += "<p id=\"averageAccel\">Average Acceleration: 0</p>";
    html += "<script>function submitWeight() { var weight = document.getElementById('weight').value; var xhr = new XMLHttpRequest(); xhr.open('POST', '/setWeight', true); xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); xhr.onreadystatechange = function() { if (xhr.readyState == 4 && xhr.status == 200) { alert('Weight updated to ' + weight + ' kg'); } }; xhr.send('weight=' + weight); }";
    html += "function fetchAverageAccel() { var xhr = new XMLHttpRequest(); xhr.open('GET', '/averageAccel', true); xhr.onreadystatechange = function() { if (xhr.readyState == 4 && xhr.status == 200) { document.getElementById('averageAccel').innerText = 'Average Acceleration: ' + xhr.responseText; } }; xhr.send(); }";
    html += "setInterval(fetchAverageAccel, 1000);</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleSetWeight() {
    if (server.hasArg("weight")) {
        weight = server.arg("weight").toFloat();
        server.send(200, "text/plain", "Weight updated to " + String(weight) + " kg");
    } else {
        server.send(400, "text/plain", "Weight value missing");
    }
}

void handleAverageAccel() {
    server.send(200, "text/plain", String(averageAccel));
}

void setup() {
    Wire.begin();
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    Serial.begin(9600);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Start the server
    server.on("/", handleRoot);
    server.on("/setWeight", HTTP_POST, handleSetWeight);
    server.on("/averageAccel", handleAverageAccel);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();

    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 14, true);

    int AcXoff = -250;
    int AcYoff = 36;
    int AcZoff = 1200;
    int GyXoff = -335;
    int GyYoff = 250;
    int GyZoff = 170;

    // Read accel data and apply correction
    AcX = (Wire.read() << 8 | Wire.read()) + AcXoff;
    AcY = (Wire.read() << 8 | Wire.read()) + AcYoff;
    AcZ = (Wire.read() << 8 | Wire.read()) + AcZoff;

    // Read gyro data & apply correction
    GyX = (Wire.read() << 8 | Wire.read()) + GyXoff;
    GyY = (Wire.read() << 8 | Wire.read()) + GyYoff;
    GyZ = (Wire.read() << 8 | Wire.read()) + GyZoff;

    // Get pitch/roll
    convert_accel(AcX, AcY, AcZ);
    acceleration_finder(AcX, AcY, AcZ);

    if (total_accel > punchThreshold && !punchActive) {
        punchActive = true;
        punchStartTime = millis();
        totalAccelDuringPunch = 0; // Reset total acceleration for new punch
        punchCount++;
    }

    if (punchActive) {
        totalAccelDuringPunch += total_accel;
        accelCount++;
    }

    if (total_accel < punchThreshold && punchActive) {
        punchActive = false;
        punchEndTime = millis();
        float punchDuration = (punchEndTime - punchStartTime) / 1000.0; // Duration in seconds
        averageAccel = totalAccelDuringPunch / accelCount;

        delay(3500);
    }

    delay(10);
}
