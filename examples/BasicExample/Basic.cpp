#include <Arduino.h>
#include "Esp.h"
#include "FMDataClient.h"

//Wifi SSID
//Wifi PsK pass
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
const char *host = "xxxx.fmi.filemaker-cloud.com";
const int port = 443;
const char *ssid = "xxxx"; 
const char *psk = "xxxx";  
const char *database = "xxxx";
const char *userName = "xxxx";
const char *password = "xxxx";
const char *layout = "xxxx";
WiFiClientSecure wifi;
UserCredentials dC(database, userName, password);
FMDataClient client(wifi, dC, host, cert, port);

String uint64ToString(uint64_t input)
{
  String result = "";
  uint8_t base = 10;

  do
  {
    char c = input % base;
    input /= base;

    if (c < 10)
      c += '0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

String chipid = uint64ToString(ESP.getEfuseMac());

void wifiConnect()
{
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, psk);
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
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
  Serial.printf("ID: %s", chipid.c_str());
  client.logInToDatabaseSession();
  Serial.printf("Response     logInToDatabaseSession: %s\n", client.getToken().c_str());
  //----------------------------------------------------------------------
  /* delay(2000);
  vector<RecordField> fields;
  RecordField f1("field1","12",FieldTypes::Text);
  RecordField f2("field2","sibr", FieldTypes::Text);
  fields.push_back(f1);
  fields.push_back(f2);
  res = client.createRecord(client.getToken(), database,layout,fields);
  Serial.printf("Response               createRecord: %s\n", res.c_str()); */
  //----------------------------------------------------------------------
  /* delay(2000);
  RecordField f3("field3","sibr", FieldTypes::Text);
  fields.push_back(f3);
  res = client.createRecord(client.getToken(), database,layout,fields);
  Serial.printf("Response               createRecord: %s\n", res.c_str()); */
  //----------------------------------------------------------------------
  /* delay(2000);
  res = client.logOutDatabaseSession();
  Serial.printf("Response      logOutDatabaseSession: %s\n", res.c_str()); */
  //----------------------------------------------------------------------
  delay(2000);
  Serial.println("--------------------------");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    wifiConnect();
  }
  /*
  vector<RecordField> recordFields;
  RecordField field1("field1", "data1", FieldTypes::Text);
  RecordField field2("field2", "data2", FieldTypes::Text);
  recordFields.push_back(field1);
  recordFields.push_back(field2);
*/
  ScriptParameters params("testScript", "1",
                                                  "testScript", "2",
                                                  "testScript", "3");
  /*
  String res = client.createRecord(client.getToken(), database, layout, recordFields, params);
  
  String res = client.uploadContainerData(database, layout, "1", "container", 1, "THis is a sample Content Bruno Silva", "data.txt", "text/plain");
*/

  vector<FindCriteria*> findCriterias;
  vector<RecordFindCriteria*> records;
  RecordFindCriteria fr1("field1","data11");
  records.push_back(&fr1);
  FindCriteria f1(records);
  findCriterias.push_back(&f1);
  RecordSortCriteria s1("CreationTimestamp",SortOrder::descend);
  vector<RecordSortCriteria*> sRecords;
  sRecords.push_back(&s1);
  SortCriteria sort(sRecords); 
  String res = client.performFind(client.getToken(), database, layout, findCriterias, 100, 0, &sort, &params);

  Serial.print(res == EMPTY_STRING ? " " : ".");
  delay(20000);
}
