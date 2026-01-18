#include <Arduino.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <Preferences.h>

// Configuration constants
const int WIFI_CONNECT_TIMEOUT_ATTEMPTS = 20;
const int WIFI_CONNECT_DELAY_MS = 500;
const unsigned long RESTART_DELAY_MS = 2000;
const unsigned long WIFI_CHECK_INTERVAL_MS = 10000; // Check WiFi every 10 seconds
const int WIFI_RECONNECT_ATTEMPTS = 3;
const int MIN_SSID_LENGTH = 1;
const int MAX_SSID_LENGTH = 32;
const int MIN_PASSWORD_LENGTH = 8;
const int MAX_PASSWORD_LENGTH = 63;

String ssid = "";
String password = "";

// AP mode SSID (will be appended with chip ID)
String ap_ssid;

AsyncWebServer server(80);
bool apMode = false;
Preferences preferences;
bool shouldRestart = false;
unsigned long restartTime = 0;
unsigned long lastWiFiCheck = 0;
int reconnectAttempts = 0;

// Function to validate WiFi credentials
bool validateCredentials(const String& ssid, const String& password, String& errorMsg)
{
  // Validate SSID
  if (ssid.length() < MIN_SSID_LENGTH || ssid.length() > MAX_SSID_LENGTH)
  {
    errorMsg.reserve(60);
    errorMsg = "SSID must be between ";
    errorMsg += MIN_SSID_LENGTH;
    errorMsg += " and ";
    errorMsg += MAX_SSID_LENGTH;
    errorMsg += " characters";
    return false;
  }

  // Validate password (empty password is allowed for open networks)
  if (password.length() > 0 && (password.length() < MIN_PASSWORD_LENGTH || password.length() > MAX_PASSWORD_LENGTH))
  {
    errorMsg.reserve(70);
    errorMsg = "Password must be between ";
    errorMsg += MIN_PASSWORD_LENGTH;
    errorMsg += " and ";
    errorMsg += MAX_PASSWORD_LENGTH;
    errorMsg += " characters";
    return false;
  }

  return true;
}

// Function to load WiFi credentials from Preferences
bool loadWiFiCredentials()
{
  if (!preferences.begin("wifi", true)) // Open in read-only mode
  {
    Serial.println("ERROR: Failed to open Preferences for reading");
    return false;
  }
  
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();

  if (ssid.length() > 0)
  {
    Serial.println("WiFi credentials loaded from Preferences");
    Serial.println("SSID: " + ssid);
    return true;
  }
  
  Serial.println("No WiFi credentials found");
  return false;
}

// Function to save WiFi credentials to Preferences
bool saveWiFiCredentials(const String& newSsid, const String& newPassword)
{
  if (!preferences.begin("wifi", false)) // Open in read-write mode
  {
    Serial.println("ERROR: Failed to open Preferences for writing");
    return false;
  }
  
  size_t ssidBytes = preferences.putString("ssid", newSsid);
  size_t passwordBytes = preferences.putString("password", newPassword);
  preferences.end();
  
  if (ssidBytes == 0)
  {
    Serial.println("ERROR: Failed to save SSID to Preferences");
    return false;
  }
  
  if (passwordBytes == 0 && newPassword.length() > 0)
  {
    Serial.println("ERROR: Failed to save password to Preferences");
    return false;
  }
  
  Serial.println("WiFi credentials saved to Preferences");
  return true;
}

// Function to start AP mode
void startAPMode()
{
  Serial.println("Starting AP mode...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid.c_str());
  Serial.println("AP Mode started");
  Serial.println("SSID: " + ap_ssid);
  Serial.println("Password: (none - open network)");
  Serial.println("IP Address: " + WiFi.softAPIP().toString());
  apMode = true;
}

// Function to check and reconnect WiFi if disconnected
void checkWiFiConnection()
{
  // Only check if we're in STA mode and not in AP mode
  if (apMode || ssid.length() == 0)
  {
    return;
  }

  // Check WiFi status periodically
  if (millis() - lastWiFiCheck >= WIFI_CHECK_INTERVAL_MS)
  {
    lastWiFiCheck = millis();

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("\nWiFi disconnected! Attempting to reconnect...");
      reconnectAttempts++;

      if (reconnectAttempts <= WIFI_RECONNECT_ATTEMPTS)
      {
        Serial.print("Reconnect attempt ");
        Serial.print(reconnectAttempts);
        Serial.print("/");
        Serial.println(WIFI_RECONNECT_ATTEMPTS);
        
        WiFi.disconnect();
        delay(100);
        WiFi.begin(ssid.c_str(), password.c_str());
        
        // Wait for connection
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < WIFI_CONNECT_TIMEOUT_ATTEMPTS)
        {
          delay(WIFI_CONNECT_DELAY_MS);
          Serial.print(".");
          attempts++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
          Serial.println("\nReconnected to WiFi!");
          Serial.println("IP Address: " + WiFi.localIP().toString());
          reconnectAttempts = 0; // Reset counter on success
        }
        else
        {
          Serial.println("\nReconnection failed.");
        }
      }
      else
      {
        Serial.println("Max reconnection attempts reached. Switching to AP mode...");
        startAPMode();
        reconnectAttempts = 0;
      }
    }
    else
    {
      // Reset reconnect attempts counter when connected
      reconnectAttempts = 0;
    }
  }
}

// Function to setup server routes
void setupServerRoutes()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              if (apMode) {
                request->redirect("/wifi_setup.html");
              } else {
                request->send(LittleFS, "/index.html", "text/html");
              }
            });

  server.on("/wifi_setup.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/wifi_setup.html", "text/html"); });

  server.on("/save_wifi_setup", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
                String newSsid = request->getParam("ssid", true)->value();
                String newPassword = request->getParam("password", true)->value();
                
                // Validate credentials
                String errorMsg;
                if (!validateCredentials(newSsid, newPassword, errorMsg))
                {
                  Serial.println("Validation failed: " + errorMsg);
                  request->send(400, "text/plain", "Validation error: " + errorMsg);
                  return;
                }
                
                // Update global variables
                ssid = newSsid;
                password = newPassword;
                
                // Save to Preferences
                if (!saveWiFiCredentials(ssid, password))
                {
                  Serial.println("Failed to save credentials to Preferences");
                  request->send(500, "text/plain", "Error: Failed to save credentials");
                  return;
                }
                
                Serial.println("New WiFi credentials received:");
                Serial.println("SSID: " + ssid);
                
                request->send(200, "text/plain", "WiFi credentials saved! Restarting...");
                
                // Schedule restart (non-blocking)
                shouldRestart = true;
                restartTime = millis() + RESTART_DELAY_MS;
              } else {
                request->send(400, "text/plain", "Missing parameters");
              } });
}

void setup()
{
  Serial.begin(115200);

  // Pre-allocate String memory to prevent heap fragmentation
  ssid.reserve(MAX_SSID_LENGTH + 1);
  password.reserve(MAX_PASSWORD_LENGTH + 1);
  ap_ssid.reserve(40); // ESP32-Setup- + 16 hex chars

  // Mount LittleFS
  if (!LittleFS.begin(true))
  {
    Serial.println("An error has occurred while mounting LittleFS");
    return;
  }
  Serial.println("LittleFS mounted successfully");

  // Create unique AP SSID with chip ID
  uint64_t chipid = ESP.getEfuseMac();
  ap_ssid = "ESP32-Setup-" + String((uint32_t)(chipid >> 32), HEX) + String((uint32_t)chipid, HEX);
  ap_ssid.toUpperCase();

  if (loadWiFiCredentials())
  {
    // Try to connect to saved WiFi
    Serial.println("Attempting to connect to saved WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_CONNECT_TIMEOUT_ATTEMPTS)
    {
      delay(WIFI_CONNECT_DELAY_MS);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nConnected to WiFi");
      Serial.println("IP Address: " + WiFi.localIP().toString());
      apMode = false;
    }
    else
    {
      Serial.println("\nFailed to connect to WiFi.");
      startAPMode();
    }
  }
  else
  {
    // No credentials saved, start in AP mode
    Serial.println("No saved WiFi credentials.");
    startAPMode();
  }

  // Setup web server routes
  setupServerRoutes();

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  // Handle scheduled restart
  if (shouldRestart && millis() >= restartTime)
  {
    ESP.restart();
  }

  // Check and maintain WiFi connection
  checkWiFiConnection();
  
  delay(100);
}