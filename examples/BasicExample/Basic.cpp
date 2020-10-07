#include <Arduino.h>
#include "Esp.h"
#include "FMDataClient.h"

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
const char *layout = "apm_home_monitor";
WiFiClientSecure wifi;
UserCredentials dC(database, userName, password);
FMDataClient client(wifi, dC, host, cert, port);

vector<RecordField> recordFields;
RecordField field1("label1", "*", FieldTypes::Text);
RecordField field2("value1", "*", FieldTypes::Text);

const size_t capacity = JSON_ARRAY_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 250;
StaticJsonDocument<capacity> doc;
String res = "";

String chipid;
ScriptParameters *params;
int secs = 1;
String sensor = "Timer";

void wifiConnect()
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

  Serial.print("Connected to ");
  Serial.println(ssid);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  Serial.println("Initialization");
  delay(100);
  //Initialize Wifi
  wifiConnect();
  delay(100);
  chipid = WiFi.macAddress();
  Serial.printf("ID: %s\n", chipid.c_str());
  params = new ScriptParameters("getDeviceActions", "", "getSystemTime", chipid);
  client.logInToDatabaseSession();
  Serial.printf("    Response     logInToDatabaseSession: %s\n", client.getToken().c_str());
  delay(100);
  Serial.println("-------------------------------------------------------------------------------------------");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    wifiConnect();
  }
  recordFields.clear();
  field1.fieldValue = sensor;
  field2.fieldValue = String(round(millis() / 1000));
  recordFields.push_back(field1);
  recordFields.push_back(field2);
  res = client.createRecord(client.getToken(), database, layout, recordFields, params);

  deserializeJson(doc, res);
  JsonObject response = doc
                            .getMember("response")
                            .as<JsonObject>();
  String actions = response
                       .getMember("scriptResult")
                       .as<String>();
  String time = response
                    .getMember("scriptResult.prerequest")
                    .as<String>();

  if (response.getMember("scriptError.prerequest").as<String>().equals("0"))
  {
    params = new ScriptParameters("getDeviceActions", time, "getSystemTime", chipid);
  }

  Serial.printf("Timestamp: %s\n", time.c_str());
  Serial.printf("Actions: %s\n", actions.c_str());

  deserializeJson(doc, actions);

  for (JsonObject actionObj : doc.as<JsonArray>())
  {
    String action = actionObj.getMember("action").as<String>();
    String parameter = actionObj.getMember("parameter").as<String>();
    if (action.equalsIgnoreCase("Set Period"))
    {
      secs = atoi(parameter.c_str());
    }
    else if (action.equalsIgnoreCase("Set Sensor"))
    {
      sensor = parameter;
    }
    else
    {
      Serial.printf("Unknown Action: %s\n", action.c_str());
    }
  }
  Serial.println("-------------------------------------------------------------------------------------------");
  delay(secs * 1000);
}
