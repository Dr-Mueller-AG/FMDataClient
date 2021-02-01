#include <Arduino.h>
#include "Esp.h"
#include "FMDataClient.h"
#include "ESP32_ISR_Timer.h"
#include <WebServer.h>

#ifndef MY_NET_SSID
#error Please define WiFi Network SSID
#endif
#ifndef MY_NET_PASS
#error Please define WiFi Network password
#endif
#ifndef MY_DB_USER
#error Please define database user name
#endif
#ifndef MY_DB_PASS
#error Please define database user password
#endif
#ifndef DEVICE_TYPE
#define DEVICE_TYPE "ESP32_Devkitc_V4"
#endif

#define TIMER_INTERRUPT_DEBUG 1
#define TIMER_DEFAULT_FREQ 1

// Server root CA
const char *cert =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIFADCCA+igAwIBAgIBBzANBgkqhkiG9w0BAQsFADCBjzELMAkGA1UEBhMCVVMx\n"
    "EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxJTAjBgNVBAoT\n"
    "HFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4xMjAwBgNVBAMTKVN0YXJmaWVs\n"
    "ZCBSb290IENlcnRpZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTExMDUwMzA3MDAw\n"
    "MFoXDTMxMDUwMzA3MDAwMFowgcYxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6\n"
    "b25hMRMwEQYDVQQHEwpTY290dHNkYWxlMSUwIwYDVQQKExxTdGFyZmllbGQgVGVj\n"
    "aG5vbG9naWVzLCBJbmMuMTMwMQYDVQQLEypodHRwOi8vY2VydHMuc3RhcmZpZWxk\n"
    "dGVjaC5jb20vcmVwb3NpdG9yeS8xNDAyBgNVBAMTK1N0YXJmaWVsZCBTZWN1cmUg\n"
    "Q2VydGlmaWNhdGUgQXV0aG9yaXR5IC0gRzIwggEiMA0GCSqGSIb3DQEBAQUAA4IB\n"
    "DwAwggEKAoIBAQDlkGZL7PlGcakgg77pbL9KyUhpgXVObST2yxcT+LBxWYR6ayuF\n"
    "pDS1FuXLzOlBcCykLtb6Mn3hqN6UEKwxwcDYav9ZJ6t21vwLdGu4p64/xFT0tDFE\n"
    "3ZNWjKRMXpuJyySDm+JXfbfYEh/JhW300YDxUJuHrtQLEAX7J7oobRfpDtZNuTlV\n"
    "Bv8KJAV+L8YdcmzUiymMV33a2etmGtNPp99/UsQwxaXJDgLFU793OGgGJMNmyDd+\n"
    "MB5FcSM1/5DYKp2N57CSTTx/KgqT3M0WRmX3YISLdkuRJ3MUkuDq7o8W6o0OPnYX\n"
    "v32JgIBEQ+ct4EMJddo26K3biTr1XRKOIwSDAgMBAAGjggEsMIIBKDAPBgNVHRMB\n"
    "Af8EBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAdBgNVHQ4EFgQUJUWBaFAmOD07LSy+\n"
    "zWrZtj2zZmMwHwYDVR0jBBgwFoAUfAwyH6fZMH/EfWijYqihzqsHWycwOgYIKwYB\n"
    "BQUHAQEELjAsMCoGCCsGAQUFBzABhh5odHRwOi8vb2NzcC5zdGFyZmllbGR0ZWNo\n"
    "LmNvbS8wOwYDVR0fBDQwMjAwoC6gLIYqaHR0cDovL2NybC5zdGFyZmllbGR0ZWNo\n"
    "LmNvbS9zZnJvb3QtZzIuY3JsMEwGA1UdIARFMEMwQQYEVR0gADA5MDcGCCsGAQUF\n"
    "BwIBFitodHRwczovL2NlcnRzLnN0YXJmaWVsZHRlY2guY29tL3JlcG9zaXRvcnkv\n"
    "MA0GCSqGSIb3DQEBCwUAA4IBAQBWZcr+8z8KqJOLGMfeQ2kTNCC+Tl94qGuc22pN\n"
    "QdvBE+zcMQAiXvcAngzgNGU0+bE6TkjIEoGIXFs+CFN69xpk37hQYcxTUUApS8L0\n"
    "rjpf5MqtJsxOYUPl/VemN3DOQyuwlMOS6eFfqhBJt2nk4NAfZKQrzR9voPiEJBjO\n"
    "eT2pkb9UGBOJmVQRDVXFJgt5T1ocbvlj2xSApAer+rKluYjdkf5lO6Sjeb6JTeHQ\n"
    "sPTIFwwKlhR8Cbds4cLYVdQYoKpBaXAko7nv6VrcPuuUSvC33l8Odvr7+2kDRUBQ\n"
    "7nIMpBKGgc0T0U7EPMpODdIm8QC3tKai4W56gf0wrHofx1l7\n"
    "-----END CERTIFICATE-----\n";

const char *host = "drm.fmi.filemaker-cloud.com";
const int port = 443;
const char *ssid = MY_NET_SSID;
const char *psk = MY_NET_PASS;
const char *database = "drm_iot";
const char *userName = MY_DB_USER;
const char *password = MY_DB_PASS;

WiFiClientSecure wifi;
UserCredentials dC(database, userName, password);
FMDataClient client(wifi, dC, host, cert, port);
StaticJsonDocument<1024> doc;
String chipid = "00:00:00:00:00:00";
String lastCallTimeStamp = "0";
//Script Parameters
ScriptParameters *scriptParameters;

// Init ESP32_ISR_Timer
ESP32_ISR_Timer ISR_Timer;
// Webserver http://0.0.0.0/
WebServer server(80);

void handleRoot()
{
  server.send(200, "text/plain", "hello from esp32!");
}

boolean authenticate()
{
  return !client.logInToDatabaseSession().equals(EMPTY_STRING);
}

static void wifiConnect()
{
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  // attempt to connect to Wifi network:
  WiFi.begin(ssid, psk);
  uint8_t counter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    // wait 1 second for re-trying
    counter++;
    delay(500);
    if (counter > 30)
      ESP.restart();
  }
  Serial.printf("Connected to %s\n", ssid);

  while (!authenticate())
  {
    delay(30000);
  }
}

static boolean healthCheck()
{
  //Find Criteria
  vector<FindCriteriaField *> findCreteriaFields_1;
  vector<FindCriteriaField *> findCreteriaFields_2;
  FindCriteriaField findCriteriaField_1("device_id", chipid);
  FindCriteriaField findCriteriaField_2("device_type", DEVICE_TYPE);
  FindCriteriaField findCriteriaField_3("f_modification_time_stamp", ">" + lastCallTimeStamp);
  findCreteriaFields_1.push_back(&findCriteriaField_1);
  findCreteriaFields_1.push_back(&findCriteriaField_3);
  findCreteriaFields_2.push_back(&findCriteriaField_2);
  findCreteriaFields_2.push_back(&findCriteriaField_3);
  FindCriteria findCriteria_1(findCreteriaFields_1);
  FindCriteria findCriteria_2(findCreteriaFields_2);
  vector<FindCriteria *> findCriterias;
  findCriterias.push_back(&findCriteria_1);
  findCriterias.push_back(&findCriteria_2);
  //Sort Criteria
  SortCriteriaField sortCriteriaField_1("ModificationTimestamp", SortOrder::ascend);
  vector<SortCriteriaField *> sortCriteriaFields;
  sortCriteriaFields.push_back(&sortCriteriaField_1);
  SortCriteria sortCriteria(sortCriteriaFields);

  //Do DATA API request
  String res = client.performFind(database, "device_actions", findCriterias, 100, 0, &sortCriteria, scriptParameters);
  log_d("Actions: %s", res.c_str());
  return !res.equals(EMPTY_STRING);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  Serial.println("Initialization");
  delay(100);
  //ESP32 As access point IP: 192.168.4.1
  WiFi.mode(WIFI_MODE_APSTA);                      //Access Point mode
  WiFi.softAP("ESPWebServer", "12345678"); //Password length minimum 8 char
  //Initialize Wifi
  wifiConnect();
  delay(100);
  chipid = WiFi.macAddress();
  scriptParameters = new ScriptParameters("getSystemTime", "", "HealthCheck", chipid);
  Serial.printf("ID: %s\n", chipid.c_str());

  server.on("/", handleRoot); //This is display page
  server.begin();             //Start server
  Serial.println("HTTP server started");
  Serial.println("-------------------------------------------------------------------------------------------");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    wifiConnect();
  }
  server.handleClient();
  delay(1);
}

/*
    String res = client.uploadContainerData(database, "iot_tab2", "1", "container", 1, "Bruno Silva", "test_file.txt", "text/plain");
    Serial.println("Container Upload:");
    Serial.println(res);
*/