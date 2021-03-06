/**************************************************************************************************
 *  
 *      Over The Air updates (OTA) - 13Jan21
 * 
 *      part of the BasicWebserver sketch - https://github.com/alanesq/BasicWebserver
 *                                   
 *                                   
 *      The /ota page is only available after a password has been supplied in the format:   http://x.x.x.x/?pwd=12345678                             
 *                                   
 *      If using an esp32cam module In Arduino IDE Select "ESP32 dev module" not "ESP32-cam" with PSRAM enabled
 *                     
 * 
 **************************************************************************************************

 
    To enable/disable OTA see setting at top of main sketch (#define ENABLE_OTA 1)

    Activate OTA with   http://<esp ip address>?pwd=12345678
    Then access with    http://<esp ip address>/ota

 
 **************************************************************************************************/


#if defined ESP32
  #include <Update.h>
#endif
 

// forward declarations (i.e. details of all functions in this file)
  void otaSetup();
  void handleOTA();


// ----------------------------------------------------------------
//                         -OTA setup section
// ----------------------------------------------------------------

// Called from 'setup'

void otaSetup() {

    server.on("/ota", handleOTA);

    // esp32 version (using webserver.h)
    #if defined ESP32
        server.on("/update", HTTP_POST, []() {
          server.sendHeader("Connection", "close");
          server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
          ESP.restart();
        }, []() {
          HTTPUpload& upload = server.upload();
          if (upload.status == UPLOAD_FILE_START) {
            if (serialDebug) Serial.setDebugOutput(true);
            if (serialDebug) Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin()) { //start with max available size
              if (serialDebug) Update.printError(Serial);
            }
          } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
              if (serialDebug) Update.printError(Serial);
            }
          } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
              if (serialDebug) Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
              if (serialDebug) Update.printError(Serial);
            }
            if (serialDebug) Serial.setDebugOutput(false);
          } else {
            if (serialDebug) Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
          }
        });
    #endif
    
    // esp8266 version  (using ESP8266WebServer.h)
    #if defined ESP8266
        server.on("/update", HTTP_POST, []() {
          server.sendHeader("Connection", "close");
          server.send(200, "text/plain", (Update.hasError()) ? "Update Failed!, rebooting" : "Update complete, device is rebooting...");
          delay(500);
          ESP.restart();
          delay(2000);
        }, []() {
          HTTPUpload& upload = server.upload();
          if (upload.status == UPLOAD_FILE_START) {
            if (serialDebug) Serial.setDebugOutput(true);
            WiFiUDP::stopAll();
            if (serialDebug) Serial.printf("Update: %s\n", upload.filename.c_str());
            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if (!Update.begin(maxSketchSpace)) { //start with max available size
              if (serialDebug) Update.printError(Serial);
            }
          } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
              if (serialDebug) Update.printError(Serial);
            }
          } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
              if (serialDebug) Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
              if (serialDebug) Update.printError(Serial);
            }
            if (serialDebug) Serial.setDebugOutput(false);
          }
          yield();
        });
    #endif

}


// -----------------------------------------------------------------------

void handleOTA(){

  WiFiClient client = server.client();          // open link with client

  // log page request including clients IP address
      IPAddress cip = client.remoteIP();
      //log_system_message("OTA web page requested from: " + String(cip[0]) + "." + String(cip[1]) + "." + String(cip[2]) + "." + String(cip[3]));


  // Send page html

    webheader(client);                            // add the standard html header
  
    client.write("<br><H1>Update firmware</H1><br>\n");
    client.printf("Current version =  %s, %s \n\n", stitle, sversion);
    
    client.write("<form method='POST' action='/update' enctype='multipart/form-data'>\n");
    client.write("<input type='file' style='width: 300px' name='update'>\n");
    client.write("<br><br><input type='submit' value='Update'></form><br>\n");
  
    client.write("<br><br>Device will reboot when upload complete");
    client.printf("%s <br>To disable OTA restart device<br> %s \n", colRed, colEnd);

    webfooter(client);                          // add the standard web page footer
    
                          
  // close html page
    delay(3);
    client.stop();
    
}


// ---------------------------------------------- end ----------------------------------------------
