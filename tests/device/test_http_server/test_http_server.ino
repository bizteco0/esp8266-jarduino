#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <BSTest.h>
#include <test_config.h>
#include <pgmspace.h>

BS_ENV_DECLARE();

static ESP8266WebServer server(80);
static uint32_t siteHits = 0;
static String siteData = "";

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    WiFi.persistent(false);
    WiFi.begin(STA_SSID, STA_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    MDNS.begin("etd");
    server.onNotFound([](){ server.send(404); });
    server.begin();
    BS_RUN(Serial);
}


TEST_CASE("HTTP GET Parameters", "[HTTPServer]")
{
    {
        siteHits = 0;
        server.on("/get", HTTP_GET, [](){
            siteData = "";
            for (uint8_t i=0; i<server.args(); i++){
                if(i > 0)
                    siteData += "&";
                siteData += server.argName(i) + "=" + server.arg(i);
            }
            siteHits++;
            server.send(200, "text/plain", siteData);
        });
        uint32_t startTime = millis();
        while(siteHits == 0 && (millis() - startTime) < 10000)
            server.handleClient();
        REQUIRE(siteHits > 0 && siteData.equals("var1=val with spaces&var+=some%"));
    }
}

TEST_CASE("HTTP POST Parameters", "[HTTPServer]")
{
    {
        siteHits = 0;
        server.on("/post", HTTP_POST, [](){
            siteData = "";
            for (uint8_t i=0; i<server.args(); i++){
                if(i > 0)
                    siteData += "&";
                siteData += server.argName(i) + "=" + server.arg(i);
            }
            siteHits++;
            server.send(200, "text/plain", siteData);
        });
        uint32_t startTime = millis();
        while(siteHits == 0 && (millis() - startTime) < 10000)
            server.handleClient();
        REQUIRE(siteHits > 0 && siteData.equals("var2=val with spaces"));
    }
}

TEST_CASE("HTTP GET+POST Parameters", "[HTTPServer]")
{
    {
        siteHits = 0;
        server.on("/get_and_post", HTTP_POST, [](){
            siteData = "";
            for (uint8_t i=0; i<server.args(); i++){
                if(i > 0)
                    siteData += "&";
                siteData += server.argName(i) + "=" + server.arg(i);
            }
            siteHits++;
            server.send(200, "text/plain", siteData);
        });
        uint32_t startTime = millis();
        while(siteHits == 0 && (millis() - startTime) < 10000)
            server.handleClient();
        REQUIRE(siteHits > 0 && siteData.equals("var3=val with spaces&var+=some%"));
    }
}

TEST_CASE("HTTP Upload", "[HTTPServer]")
{
    {
        siteHits = 0;
        server.on("/upload", HTTP_POST, [](){
            for (uint8_t i=0; i<server.args(); i++){
                if(i > 0)
                    siteData += "&";
                siteData += server.argName(i) + "=" + server.arg(i);
            }
            siteHits++;
            server.send(200, "text/plain", siteData);
        }, [](){
            HTTPUpload& upload = server.upload();
            if(upload.status == UPLOAD_FILE_START){
              siteData = upload.filename;
            } else if(upload.status == UPLOAD_FILE_END){
              siteData.concat(":");
              siteData.concat(String(upload.totalSize));
              siteData.concat("&");
            }
        });
        uint32_t startTime = millis();
        while(siteHits == 0 && (millis() - startTime) < 10000)
            server.handleClient();
        REQUIRE(siteHits > 0 && siteData.equals("test.txt:16&var4=val with spaces"));
    }
}

void loop()
{
}
