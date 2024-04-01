void ensure_wifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin("XXXX", "YYYY");
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("Connection Established");
}
