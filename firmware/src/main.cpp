#include<Update.h>
#include <WiFi.h>

// WiFi credentials
const char* ssid = "loop";
const char* password = "cyber@123";

// Server port
const uint16_t serverPort = 8080; // Replace with the desired port number

WiFiServer server(serverPort);

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

      Serial.printf("Server started. Access it at http://%s:%d\n", WiFi.localIP().toString().c_str(), serverPort);

  // Start the server
  server.begin();
  Serial.printf("Server started on port %d\n", serverPort);
}

void loop() {
  // Check for incoming client connections
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected");
    while (client.connected()) {
      if (client.available()) {
        // Read data from the client
        String data = client.readStringUntil('\n');
        Serial.printf("Received: %s\n", data.c_str());

        // Respond to the client
        if (data.startsWith("PING")) {
          client.println("PONG");
        } else if (data.startsWith("HELLO")) {
          client.println("Hello from ESP32!");
        } else {
          client.println("Unknown command");
        }
      }
    }

    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}
