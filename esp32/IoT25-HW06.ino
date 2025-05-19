// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "iPhone";
const char* password = "12345123";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Variable to store the current output state
String output26State = "off";

// Assign output variables to GPIO pins
const int output26 = 26;

// Timing variables
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;  // 2 seconds timeout

// Client connection state
bool clientConnected = false;

void setup() {
  Serial.begin(115200);
  pinMode(output26, OUTPUT);  // Set GPIO 26 as an output
  digitalWrite(output26, LOW); // Initialize GPIO 26 as LOW

  // Connect to Wi-Fi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print the local IP address and start the web server
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client) {
    // Only print "New Client" once per connection
    if (!clientConnected) {
      Serial.println("New Client.");
      clientConnected = true;
    }

    currentTime = millis();
    previousTime = currentTime;
    String currentLine = ""; // Stores the current line of incoming data

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);  // Print received character to serial monitor
        header += c;      // Append the character to the header string

        // If the character is a newline
        if (c == '\n') {
          // If the current line is empty, it signifies the end of the HTTP request
          if (currentLine.length() == 0) {

            // Send HTTP response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Handle GPIO control
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 ON");
              output26State = "on";
              digitalWrite(output26, HIGH);
            } 
            else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 OFF");
              output26State = "off";
              digitalWrite(output26, LOW);
            }

            // Send the HTML webpage
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>");
            client.println("body { font-family: Helvetica; text-align: center; }");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("font-size: 30px; cursor: pointer; }");
            client.println(".button2 { background-color: #555555; }");
            client.println("</style></head>");
            client.println("<body><h1>ESP32 Web Server</h1>");
            client.println("<p>GPIO 26 - State: " + output26State + "</p>");

            // Display ON/OFF buttons based on the current state
            if (output26State == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("</body></html>");
            client.println();  // End of HTTP response

            // Exit the while loop
            break;
          } 
          else {
            currentLine = "";  // Clear the current line
          }
        } 
        else if (c != '\r') {
          currentLine += c;  // Append characters to the current line
        }
      }
    }

    // Close the client connection and reset the header
    client.stop();
    header = "";
    clientConnected = false;  // Reset connection state
  }
}
