#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// GPS Config
#define RXD2 42
#define TXD2 41
#define GPS_BAUD 9600

TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // UART2

// WiFi Config
const char* ssid = "Your SSID";

AsyncWebServer server(80);

// Biến lưu vị trí GPS
float latitude = 10.762622;
float longitude = 106.660172;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 GPS Tracker</title>
    <script>
        function initMap() {
            fetch('/location')
            .then(response => response.json())
            .then(data => {
                var location = { lat: data.lat, lng: data.lng };
                var map = new google.maps.Map(document.getElementById("map"), {
                    zoom: 15,
                    center: location
                });
                new google.maps.Marker({
                    position: location,
                    map: map
                });
            });
        }
    </script>
</head>
<body>
    <h1>Vị trí xe điều khiển</h1>
    <div id="map" style="width:100%; height:500px;"></div>
    <script src="Your API Google Map" async defer></script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  // GPS begin
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);

  // Kết nối WiFi
  WiFi.begin(ssid);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối WiFi!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/location", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"lat\": " + String(latitude, 6) + ", \"lng\": " + String(longitude, 6) + "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  // Đọc dữ liệu từ GPS
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    gps.encode(c);
  }

  // Nếu có vị trí mới từ GPS
  if (gps.location.isUpdated()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();

    Serial.println("==== GPS Updated ====");
    Serial.print("LAT: "); Serial.println(latitude, 6);
    Serial.print("LONG: "); Serial.println(longitude, 6);
  }
}
