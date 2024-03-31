#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h> //https://github.com/lacamera/ESPAsyncWebServer
#include <LittleFS.h>

// Thông tin WiFi
const char* ssid = "YOUR_SSID_NAME";
const char* password = "YOUR_SSID_PASSWORD";

// Thiết lập web server trên port 80
AsyncWebServer server(80);

// Khai báo chân LM35, LED và nút nhấn
const int LM35_PIN = A0;
const int LED1 = D1;
const int LED2 = D2;
const int BUTTON1 = D5;
const int BUTTON2 = D6;

// Biến trạng thái LED và giá trị nhiệt độ
int ledState1 = LOW;
int ledState2 = LOW;
float temperature = 0;

// Biến lưu trữ log
String logData = "";

void setup() {
  // Khởi tạo Serial
  Serial.begin(115200);

  // Kết nối WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Khởi tạo LittleFS
  if (!LittleFS.begin()) {
    Serial.println("An error occurred while mounting LittleFS");
    return;
  }

  // Khởi tạo server
  server.begin();

  // Thiết lập chân LED và nút nhấn
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendHttpResponse(request);
  });

  // Serve images from LittleFS
  server.on("/bg.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/bg.svg", "image/svg+xml");
  });
  server.on("/logo_UTC2.jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/logo_UTC2.jpg", "image/jpeg");
  });
  server.on("/logo_ddt.jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/logo_ddt.jpg", "image/jpeg");
  });

  // Routes for LED control
  server.on("/bat1", HTTP_GET, [](AsyncWebServerRequest *request) {
      bat1();
      // No need to send a response
  });
  server.on("/tat1", HTTP_GET, [](AsyncWebServerRequest *request) {
      tat1();
      // No need to send a response
  });
  server.on("/bat2", HTTP_GET, [](AsyncWebServerRequest *request) {
      bat2();
      // No need to send a response
  });
  server.on("/tat2", HTTP_GET, [](AsyncWebServerRequest *request) {
      tat2();
      // No need to send a response
  });
}

int btnState1;
int btnState2;

void loop() {
  // Đọc trạng thái nút nhấn
  btnState1 = digitalRead(BUTTON1);
  btnState2 = digitalRead(BUTTON2);

  // Đọc trạng thái nút nhấn và thiết lập trạng thái LED
  if (btnState1 == LOW) {
    ledState1 = (ledState1 == LOW ? HIGH : LOW);
    digitalWrite(LED1, ledState1);
  }

  if (btnState2 == LOW) {
    ledState2 = (ledState2 == LOW ? HIGH : LOW);
    digitalWrite(LED2, ledState2);
  }

  // Đọc giá trị từ LM35
  readTemperatureValue();

  // Xuất giá trị từ LM35 ra màn hình web
  showTemperatureValue();

  // Cập nhật log
  updateLog();

  delay(300);
}

// Bat led 1
void bat1() {
  ledState1 = HIGH;
  digitalWrite(LED1, ledState1);
}

// Tat led 1
void tat1() {
  ledState1 = LOW;
  digitalWrite(LED1, ledState1);
}

// Bat led 2
void bat2() {
  ledState2 = HIGH;
  digitalWrite(LED2, ledState2);
}

// Tat led 2
void tat2() {
  ledState2 = LOW;
  digitalWrite(LED2, ledState2);
}

void readTemperatureValue() {
  // Đọc giá trị từ LM35
  temperature = (5000 * analogRead(LM35_PIN) / 1024.0) * 100.0; // Công thức tính nhiệt độ cho LM35
}

void sendHttpResponse(AsyncWebServerRequest *request) {
  // Read the index.html file from flash
  String html = readFileFromFlash("/index.html");

  // Replace the placeholder with the actual temperature value
  html.replace("{{temperature}}", showTemperatureValue());

  // Send the HTTP response
  request->send(200, "text/html", html);
}

String readFileFromFlash(String path) {
  String fileContent = "";
  File file = LittleFS.open(path, "r");
  if (file) {
    while (file.available()) {
      fileContent += (char)file.read();
    }
    file.close();
  } else {
    Serial.println("Error opening file: " + path);
  }
  return fileContent;
}

String showTemperatureValue() {
  // Generate the HTML string with the temperature value
  return String(temperature, 2);
}

void updateLog() {
  // Cập nhật log với trạng thái LED và giá trị nhiệt độ
  String newLog = "LED1: " + String(ledState1) + ", LED2: " + String(ledState2) + ", Temperature: " + String(temperature) + "°C";
  logData = newLog + "\n" + logData.substring(0, logData.length() - newLog.length() - 1);
  Serial.println(logData);
}
