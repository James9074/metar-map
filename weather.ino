String get_weather(String airports){
  // Prep Client (TODO: This probably could be reusable)
  HTTPClient https;
  https.addHeader("Content-Type", "plain/text");
  https.addHeader("Transfer-Encoding", "chunked");
    
  String urlStr = BASE_URI + airports;
  Serial.print("[HTTPS] begin...\n");
  Serial.println(urlStr.c_str());
  
  if (https.begin(*SSL_CLIENT, urlStr)) {
    int httpCode = https.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String res = https.getString();
        https.end();
        if (DEBUG){
          Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        }    
        return res;
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  return "";
}
