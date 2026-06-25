#include <WiFi.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- WIFI ----------------
const char* ssid = "Nothing 2a plus";
const char* password = "14042007";

// ---------------- WEB SERVER ----------------
WebServer server(80);

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- PIN CONFIG ----------------
#define ONE_WIRE_BUS 4
#define DHTPIN 5
#define DHTTYPE DHT22

#define GREEN_LED 18
#define YELLOW_LED 19
#define RED_LED 23

// ---------------- OBJECTS ----------------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHT dht(DHTPIN, DHTTYPE);

// ---------------- BASELINE ----------------
#define WINDOW_SIZE 10
float humidityHistory[WINDOW_SIZE];
float tempHistory[WINDOW_SIZE];

int indexVal = 0;
bool bufferFilled = false;
bool calibrationDone = false;

float baselineHumidity = 65.0;
float baselineDeltaT = 0.0;

// ---------------- VARIABLES ----------------
float Tw, Ta, H;
float deltaT, humidity_dev, deltaT_dev;

float prevDeltaT = 0;
int riseCount = 0;
int trend = 0;

String statusText = "CALIBRATING";
int riskScore = 0;

// ---------------- GRAPH HISTORY ----------------
#define HISTORY_SIZE 20
float woundTempSeries[HISTORY_SIZE];
float humiditySeries[HISTORY_SIZE];
int riskSeries[HISTORY_SIZE];
int historyIndex = 0;
bool historyFilled = false;

// ---------------- AI MODEL ----------------
String predictStatus(float deltaT_dev, float humidity_dev, int trend) {
  if (humidity_dev <= 4.5) return "NORMAL";
  else if (deltaT_dev <= 3.45) return "WARNING";
  else return "CRITICAL";
}

// ---------------- RISK ----------------
int calculateRisk(float deltaT_dev, float humidity_dev, int trend) {
  float risk = (deltaT_dev * 10) + (humidity_dev * 2) + (trend * 20);
  if (risk > 100) risk = 100;
  if (risk < 0) risk = 0;
  return (int)risk;
}

// ---------------- LED ----------------
void updateLED(String status) {
  digitalWrite(GREEN_LED, status == "NORMAL" ? HIGH : LOW);
  digitalWrite(YELLOW_LED, status == "WARNING" ? HIGH : LOW);
  digitalWrite(RED_LED, status == "CRITICAL" ? HIGH : LOW);
}

// ---------------- STORE GRAPH DATA ----------------
void storeHistory(float wt, float hum, int risk) {
  woundTempSeries[historyIndex] = wt;
  humiditySeries[historyIndex] = hum;
  riskSeries[historyIndex] = risk;

  historyIndex++;
  if (historyIndex >= HISTORY_SIZE) {
    historyIndex = 0;
    historyFilled = true;
  }
}

// ---------------- JSON FOR CHARTS ----------------
String floatArrayToJS(float arr[], int size) {
  String s = "[";
  int count = historyFilled ? size : historyIndex;
  for (int i = 0; i < count; i++) {
    int idx = historyFilled ? (historyIndex + i) % size : i;
    s += String(arr[idx], 1);
    if (i < count - 1) s += ",";
  }
  s += "]";
  return s;
}

String intArrayToJS(int arr[], int size) {
  String s = "[";
  int count = historyFilled ? size : historyIndex;
  for (int i = 0; i < count; i++) {
    int idx = historyFilled ? (historyIndex + i) % size : i;
    s += String(arr[idx]);
    if (i < count - 1) s += ",";
  }
  s += "]";
  return s;
}

// ---------------- DASHBOARD PAGE ----------------
String getHTML() {
  String statusColor = "#64748b";
  if (statusText == "NORMAL") statusColor = "#16a34a";
  else if (statusText == "WARNING") statusColor = "#eab308";
  else if (statusText == "CRITICAL") statusColor = "#dc2626";
  else if (statusText == "CALIBRATING") statusColor = "#2563eb";

  String woundJS = floatArrayToJS(woundTempSeries, HISTORY_SIZE);
  String humJS = floatArrayToJS(humiditySeries, HISTORY_SIZE);
  String riskJS = intArrayToJS(riskSeries, HISTORY_SIZE);
  int pointsCount = historyFilled ? HISTORY_SIZE : historyIndex;

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Wound Monitor</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<style>
body{
  margin:0;
  font-family:Arial,sans-serif;
  background:#f3f6fb;
  color:#111827;
}
.container{
  max-width:950px;
  margin:0 auto;
  padding:18px;
}
.header{
  display:flex;
  justify-content:space-between;
  align-items:center;
  margin-bottom:18px;
  flex-wrap:wrap;
  gap:12px;
}
.title{
  font-size:28px;
  font-weight:700;
}
.sub{
  color:#6b7280;
  font-size:14px;
}
.status{
  padding:14px 22px;
  border-radius:16px;
  color:white;
  font-weight:700;
  font-size:18px;
  background:)rawliteral" + statusColor + R"rawliteral(;
}
.grid{
  display:grid;
  grid-template-columns:repeat(auto-fit,minmax(180px,1fr));
  gap:14px;
  margin-bottom:18px;
}
.card{
  background:white;
  border-radius:18px;
  padding:16px;
  box-shadow:0 4px 16px rgba(0,0,0,0.08);
}
.label{
  color:#6b7280;
  font-size:13px;
  margin-bottom:8px;
}
.value{
  font-size:24px;
  font-weight:700;
}
.chart-card{
  background:white;
  border-radius:18px;
  padding:16px;
  box-shadow:0 4px 16px rgba(0,0,0,0.08);
  margin-bottom:16px;
}
.footer{
  text-align:center;
  color:#6b7280;
  font-size:12px;
  margin-top:10px;
}
canvas{
  width:100% !important;
  height:280px !important;
}
</style>
<script>
setTimeout(function(){ location.reload(); }, 2000);
</script>
</head>
<body>
<div class="container">
  <div class="header">
    <div>
      <div class="title">Wound Monitoring Dashboard</div>
      <div class="sub">ESP32 Edge AI System</div>
    </div>
    <div class="status">)rawliteral" + statusText + R"rawliteral(</div>
  </div>

  <div class="grid">
    <div class="card"><div class="label">Risk Score</div><div class="value">)rawliteral" + String(riskScore) + R"rawliteral(/100</div></div>
    <div class="card"><div class="label">Wound Temp</div><div class="value">)rawliteral" + String(Tw,1) + R"rawliteral( °C</div></div>
    <div class="card"><div class="label">Ambient Temp</div><div class="value">)rawliteral" + String(Ta,1) + R"rawliteral( °C</div></div>
    <div class="card"><div class="label">Humidity</div><div class="value">)rawliteral" + String(H,1) + R"rawliteral( %</div></div>
    <div class="card"><div class="label">DeltaT</div><div class="value">)rawliteral" + String(deltaT,2) + R"rawliteral(</div></div>
    <div class="card"><div class="label">DeltaT Dev</div><div class="value">)rawliteral" + String(deltaT_dev,2) + R"rawliteral(</div></div>
    <div class="card"><div class="label">Humidity Dev</div><div class="value">)rawliteral" + String(humidity_dev,2) + R"rawliteral(</div></div>
    <div class="card"><div class="label">Trend</div><div class="value">)rawliteral" + String(trend) + R"rawliteral(</div></div>
    <div class="card"><div class="label">Baseline H</div><div class="value">)rawliteral" + String(baselineHumidity,1) + R"rawliteral(</div></div>
    <div class="card"><div class="label">Baseline dT</div><div class="value">)rawliteral" + String(baselineDeltaT,2) + R"rawliteral(</div></div>
  </div>

  <div class="chart-card">
    <div class="label">Wound Temperature Trend</div>
    <canvas id="woundChart"></canvas>
  </div>

  <div class="chart-card">
    <div class="label">Humidity Trend</div>
    <canvas id="humidityChart"></canvas>
  </div>

  <div class="chart-card">
    <div class="label">Risk Score Trend</div>
    <canvas id="riskChart"></canvas>
  </div>

  <div class="footer">Live refresh every 2 seconds</div>
</div>

<script>
const labels = Array.from({length: )rawliteral" + String(pointsCount) + R"rawliteral(}, (_, i) => i + 1);

new Chart(document.getElementById('woundChart'), {
  type: 'line',
  data: {
    labels: labels,
    datasets: [{
      label: 'Wound Temp',
      data: )rawliteral" + woundJS + R"rawliteral(,
      borderWidth: 2,
      tension: 0.35
    }]
  },
  options: { responsive: true, maintainAspectRatio: false }
});

new Chart(document.getElementById('humidityChart'), {
  type: 'line',
  data: {
    labels: labels,
    datasets: [{
      label: 'Humidity',
      data: )rawliteral" + humJS + R"rawliteral(,
      borderWidth: 2,
      tension: 0.35
    }]
  },
  options: { responsive: true, maintainAspectRatio: false }
});

new Chart(document.getElementById('riskChart'), {
  type: 'line',
  data: {
    labels: labels,
    datasets: [{
      label: 'Risk Score',
      data: )rawliteral" + riskJS + R"rawliteral(,
      borderWidth: 2,
      tension: 0.35
    }]
  },
  options: { responsive: true, maintainAspectRatio: false }
});
</script>
</body>
</html>
)rawliteral";

  return html;
}

// ---------------- WEB ----------------
void handleRoot() {
  server.send(200, "text/html", getHTML());
}

// ---------------- SYSTEM UPDATE ----------------
void updateSystem() {
  sensors.requestTemperatures();
  Tw = sensors.getTempCByIndex(0);
  Ta = sensors.getTempCByIndex(1);
  H = dht.readHumidity();

  if (isnan(H) || Tw == -127 || Ta == -127) {
    Serial.println("Sensor Error!");
    return;
  }

  deltaT = Tw - Ta;

  humidityHistory[indexVal] = H;
  tempHistory[indexVal] = deltaT;

  indexVal++;
  if (indexVal >= WINDOW_SIZE) {
    indexVal = 0;
    bufferFilled = true;
  }

  // ---- CALIBRATION ----
  if (!bufferFilled) {
    statusText = "CALIBRATING";
    riskScore = 0;

    Serial.println("Calibrating... Please wait");
    Serial.print("Humidity: "); Serial.println(H);
    Serial.print("DeltaT: "); Serial.println(deltaT);
    Serial.println("-------------------------");

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.println("Calibrating...");
    display.display();

    updateLED("OFF");
    return;
  }

  float sumH = 0, sumT = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sumH += humidityHistory[i];
    sumT += tempHistory[i];
  }

  baselineHumidity = sumH / WINDOW_SIZE;
  baselineDeltaT = sumT / WINDOW_SIZE;

  if (!calibrationDone) {
    Serial.println("Calibration Complete ✅");
    Serial.print("Baseline Humidity: ");
    Serial.println(baselineHumidity);
    Serial.print("Baseline DeltaT: ");
    Serial.println(baselineDeltaT);
    Serial.println("=========================");
    calibrationDone = true;
  }

  humidity_dev = H - baselineHumidity;
  deltaT_dev = deltaT - baselineDeltaT;

  if (deltaT_dev > prevDeltaT) riseCount++;
  else riseCount = 0;

  trend = (riseCount >= 3) ? 1 : 0;
  prevDeltaT = deltaT_dev;

  statusText = predictStatus(deltaT_dev, humidity_dev, trend);
  riskScore = calculateRisk(deltaT_dev, humidity_dev, trend);

  updateLED(statusText);
  storeHistory(Tw, H, riskScore);

  // ---- SERIAL OUTPUT ----
  Serial.println("---------------");
  Serial.print("Tw: "); Serial.println(Tw);
  Serial.print("Ta: "); Serial.println(Ta);
  Serial.print("Humidity: "); Serial.println(H);

  Serial.print("Baseline Humidity: "); Serial.println(baselineHumidity);
  Serial.print("Baseline DeltaT: "); Serial.println(baselineDeltaT);

  Serial.print("DeltaT: "); Serial.println(deltaT);
  Serial.print("DeltaT_dev: "); Serial.println(deltaT_dev);
  Serial.print("Humidity_dev: "); Serial.println(humidity_dev);

  Serial.print("Trend: "); Serial.println(trend);
  Serial.print("STATUS: "); Serial.println(statusText);
  Serial.print("Risk Score: "); Serial.println(riskScore);
  Serial.println("=========================");

  // ---- OLED (ONLY STATUS + RISK) ----
  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Status:");

  display.setTextSize(3);
  display.setCursor(0, 20);
  display.println(statusText);

  display.setTextSize(2);
  display.setCursor(0, 50);
  display.print("Risk:");
  display.print(riskScore);

  display.display();
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  sensors.begin();
  dht.begin();

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  Serial.println("System Starting...");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected ✅");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();

  Serial.println("Web Dashboard Started ✅");
}

// ---------------- LOOP ----------------
void loop() {
  server.handleClient();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 2000) {
    lastUpdate = millis();
    updateSystem();
  }
}