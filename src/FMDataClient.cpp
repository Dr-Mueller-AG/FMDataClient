/*
  FMDataClient.h - Filemaker DATA API library 
  Copyright (c) 2020 Bruno Silva.  DrM, Dr. Mueller AG.
*/

#include "FMDataClient.h"

DatabaseCredentials::DatabaseCredentials(String database)
{
  if (database == EMPTY_STRING)
    throw ERROR_MSG_EMPTY_DATABASE_NAME;
  this->database = database;
}

DatabaseCredentials::DatabaseCredentials(
    String database,
    const vector<DatabaseCredentials *> &externalDatabasesCredentials)
    : DatabaseCredentials(database)
{
  this->externalDatabasesCredentials = externalDatabasesCredentials;
}

String DatabaseCredentials::getExternalDatabasesCredentialsPayload(void) const
{
  String result = EMPTY_STRING;
  if (!externalDatabasesCredentials.empty())
  {
    log_d("External Databases Credentials found.");
    int size = externalDatabasesCredentials.size();
    const size_t capacity = JSON_ARRAY_SIZE(size) +
                            JSON_OBJECT_SIZE(1) +
                            size * JSON_OBJECT_SIZE(4);
    DynamicJsonDocument doc(capacity);
    JsonArray fmDataSource = doc.createNestedArray(PARAMETER_FM_DATA_SOURCE);
    for (DatabaseCredentials *c : externalDatabasesCredentials)
    {
      log_d("External Database: %s", c->database.c_str());
      fmDataSource.add(c->toJSON());
    }
    serializeJson(doc, result);
  }
  log_d("External Databases Credentials: %s", result.c_str());
  return result;
}

/**
 * @brief Creates the Authorization Header when using the User Credentials
 * 
 * @return String 
 */
String UserCredentials::getAuthorizationHeaderValue(void) const
{
  return base64::encode(this->userName + String(":") + this->password);
}

UserCredentials::UserCredentials(
    String database,
    String userName,
    String password) : DatabaseCredentials(database)
{
  if (userName == EMPTY_STRING)
    throw ERROR_MSG_EMPTY_USER_NAME;
  if (password == EMPTY_STRING)
    throw ERROR_MSG_EMPTY_PASSWORD;
  this->userName = userName;
  this->password = password;
}

UserCredentials::UserCredentials(
    String database,
    String userName,
    String password,
    const vector<DatabaseCredentials *> &externalDatabasesCredentials)
    : DatabaseCredentials(
          database,
          externalDatabasesCredentials)
{
  if (userName == EMPTY_STRING)
    throw ERROR_MSG_EMPTY_USER_NAME;
  if (password == EMPTY_STRING)
    throw ERROR_MSG_EMPTY_PASSWORD;
  this->userName = userName;
  this->password = password;
}

JsonObject UserCredentials::toJSON(void)
{
  const size_t CAPACITY = JSON_OBJECT_SIZE(4);
  StaticJsonDocument<CAPACITY> doc;
  JsonObject obj = doc.to<JsonObject>();
  obj[PARAMETER_DATABASE] = this->database;
  obj[PARAMETER_USER_NAME] = this->userName;
  obj[PARAMETER_PASSWORD] = this->password;
  return obj;
}

OAuthUserCredentials::OAuthUserCredentials(String database, String oAuthRequestId, String oAuthId)
    : DatabaseCredentials(database)
{
  if (oAuthRequestId == EMPTY_STRING)
    throw ERROR_MSG_EMPTY_OAUTH_REQUEST_ID;
  if (oAuthId == EMPTY_STRING)
    throw ERROR_MSG_EMPTY_OAUTH_ID;
  this->oAuthId = oAuthId;
  this->oAuthRequestId = oAuthRequestId;
}

OAuthUserCredentials::OAuthUserCredentials(
    String database,
    String oAuthRequestId,
    String oAuthId,
    const vector<DatabaseCredentials *> &externalDatabasesCredentials)
    : DatabaseCredentials(
          database,
          externalDatabasesCredentials)
{
  if (oAuthRequestId == EMPTY_STRING)
    throw ERROR_MSG_EMPTY_OAUTH_REQUEST_ID;
  if (oAuthId == EMPTY_STRING)
    throw ERROR_MSG_EMPTY_OAUTH_ID;
  this->oAuthId = oAuthId;
  this->oAuthRequestId = oAuthRequestId;
}

CredentialsType OAuthUserCredentials::getType(void) const
{
  return CredentialsType::OAuthUserCredentialsType;
}

CredentialsType UserCredentials::getType(void) const
{
  return CredentialsType::UserCredentialsType;
}

JsonObject OAuthUserCredentials::toJSON(void)
{
  const size_t CAPACITY = JSON_OBJECT_SIZE(4);
  StaticJsonDocument<CAPACITY> doc;
  JsonObject obj = doc.to<JsonObject>();
  obj[PARAMETER_DATABASE] = database;
  obj[PARAMETER_OAUTH_REQUEST_ID] = this->oAuthRequestId;
  obj[PARAMETER_OAUTH_IDENTIFIER] = this->oAuthId;
  return obj;
}

RecordField::RecordField(String fieldName, String fieldValue, FieldTypes fieldType)
{
  this->fieldName = fieldName;
  this->fieldType = fieldType;
  this->fieldValue = fieldValue;
}

RecordField::RecordField(String fieldName, float fieldValue)
{
  this->fieldName = fieldName;
  this->fieldType = FieldTypes::Number;
  this->fieldValue = String(fieldValue);
}

size_t RecordField::getSize()
{
  return this->fieldValue.length() + this->fieldName.length() + 1;
}

RecordField::RecordField(String fieldName, int fieldValue)
{
  this->fieldName = fieldName;
  this->fieldType = FieldTypes::Number;
  this->fieldValue = String(fieldValue);
}
String OAuthUserCredentials::getAuthorizationHeaderValue(void) const
{
  throw ERROR_MSG_NOT_IMPLEMENTED;
}

String DatabaseCredentials::getLogInUrl(void) const
{
  size_t length = sizeof(URL_SESSIONS) + this->database.length();
  char buff[length];
  snprintf(buff, length, URL_SESSIONS, this->database.c_str());
  return String(buff);
}

String DatabaseCredentials::getLogOutUrl(const String &token) const
{
  size_t length = sizeof(URL_SESSION_DELETE) + this->database.length() + token.length();
  char buff[length];
  snprintf(buff, length, URL_SESSION_DELETE, this->database.c_str(), token.c_str());
  return String(buff);
}
/**
 * @brief 
 * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-out
 * @return String 
 */
String FMDataClient::logOutDatabaseSession(void)
{
  if (this->_token == EMPTY_STRING || this->_token == NULL)
  {
    log_d(ERROR_MSG_EMPTY_TOKEN);
    return EMPTY_STRING;
  }
  if (!this->_https.begin(
          this->_client,
          this->_host,
          this->_port,
          this->_credentials->getLogOutUrl(this->_token),
          true))
  {
    log_e("Could not connect to: %s", this->_host.c_str());
    return EMPTY_STRING;
  }
  this->_https.setAuthorization(EMPTY_STRING);
  this->_https.setUserAgent(HEADER_AGENT_VALUE);
  this->_https.setReuse(false);
  this->_https.addHeader(HEADER_CONTENT_TYPE, MIME_TYPE_APPLICATION_JSON);
  int httpCode = this->_https.sendRequest(HTTP_METHOD_DELETE, String(EMPTY_STRING));
  const String &response = this->_https.getString();
  this->_https.end();
  if (httpCode == HTTP_CODE_OK)
  {
    log_d("Successfull request - Status: %d", httpCode);
    const size_t capacity = // JSON Memory Size
        JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(0) + 2 * JSON_OBJECT_SIZE(2) + 116;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, response);
    if (error)
    {
      log_e("ArduinoJson error - %s", error.c_str());
      return EMPTY_STRING;
    }
    else
    {
      JsonArray messages = doc[PARAMETER_MESSAGES].as<JsonArray>();
      for (JsonObject msg : messages)
      {
        if (msg[PARAMETER_CODE].as<int>() != 0)
        {
          log_e("Filemaker error: %d - %s",
                msg[PARAMETER_CODE].as<int>(),
                msg[PARAMETER_MESSAGE].as<const char *>());
          return EMPTY_STRING;
        }
      }
      return response;
    }
  }
  else
  {
    log_e("Http error: %d - %s", httpCode, this->_https.errorToString(httpCode));
  }
  return EMPTY_STRING;
}
/**
   * @brief Get the list of supported OAuth providers and tracking Id
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-in-oauth
   * @return String Filemaker response
   */
String FMDataClient::getListOfOAuthProviders(void)
{
  try
  {
    this->_client.stop();
    this->_https.begin(
        this->_client,
        this->_host,
        this->_port,
        URL_OAUTH_PROVIDERS);
    this->_https.setUserAgent(HEADER_AGENT_VALUE);
    this->_https.addHeader(HEADER_HOST, this->_host);
    this->_https.addHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
    int httpCode = this->_https.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      const String &response = this->_https.getString();
      this->_https.end();
      return response;
    }
    else
    {
      if (this->_https.connected())
      {
        this->_https.end();
      }
      throw this->_https.errorToString(httpCode);
    }
    return EMPTY_STRING;
  }
  catch (const char *msg)
  {
    if (this->_https.connected())
    {
      this->_https.end();
    }
    return F(msg);
  }
  catch (...)
  {
    if (this->_https.connected())
    {
      this->_https.end();
    }
    return ERROR_MSG_UNKNOWN_ERROR;
  }
}

/**
 * @brief 
 * 
 * @param trackingId 
 * @param oauthProvider 
 * @param address 
 * @param oauthType 
 * @return String 
 */
String FMDataClient::getOAuthRequestId(String trackingId, String oauthProvider, String address, int oauthType)
{
  throw ERROR_MSG_NOT_IMPLEMENTED;
}

/**
   * @brief Create a record
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_create-record
   * @param token The Authentication Token
   * @param database Database Name
   * @param layout Layout Name
   * @param fields List of fields with values
   * @return String Json with result or empty string when it fails
   */
String FMDataClient::createRecord(String token, String database, String layout, vector<RecordField> fields)
{

  String url(stringf(URL_RECORD_NEW, database.c_str(), layout.c_str()));
  log_d("Url: %s", url.c_str());
  String payload = generateCreatePayload(fields);
  log_d("Payload: %s", payload.c_str());
  String auth = generateAuth(token.c_str());
  log_d("Authorization: %s", auth.c_str());

  if (!this->_https.begin(
          this->_client,
          this->_host,
          443,
          url, true))
  {
    log_e("Could not connect to: %s", this->_host.c_str());
    return EMPTY_STRING;
  }
  this->_https.setAuthorization(EMPTY_STRING);
  this->_https.setUserAgent(HEADER_AGENT_VALUE);
  this->_https.setReuse(false);
  this->_https.addHeader(HEADER_AUTHORIZATION, auth.c_str());
  this->_https.addHeader(HEADER_ACCEPT, HEADER_ACCEPT_VALUE_ALL);
  this->_https.addHeader(HEADER_CACHE_CONTROL, HEADER_CACHE_CONTROL_VALUE_NO_CACHE);
  this->_https.addHeader(HEADER_CONTENT_TYPE, MIME_TYPE_APPLICATION_JSON);
  int httpCode = this->_https.POST(payload);
  const String &response = this->_https.getString();
  log_d("Response: %s", response.c_str());
  this->_https.end();
  if (httpCode == HTTP_CODE_OK)
  {
    log_d("Successfull request - Status: %d", httpCode);
    return response;
  }
  else
  {
    log_e("Http error: %d - %s", httpCode, this->_https.errorToString(httpCode));
  }
  return EMPTY_STRING;
}
/**
   * @brief Create a Record object
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_create-record
   * @param database Database Name
   * @param layout Layout Name
   * @param fields List of fields with values
   * @return String Json with result or empty string when it fails
   */
String FMDataClient::createRecord(String database, String layout, vector<RecordField> fields)
{
  if (this->_token == EMPTY_STRING)
  {
    log_e("Error token is empty");
    return EMPTY_STRING;
  }
  else
  {
    return this->createRecord(this->_token, database, layout, fields);
  }
}
String FMDataClient::generateAuth(const char *token)
{
  String result = String(PARAMETER_BEARER) + String(token);
  return result;
}

/**
 * @brief 
 * 
 * @param token 
 * @param database 
 * @param layout 
 * @param recordId 
 * @param fields 
 * @return String 
 */
String FMDataClient::editRecord(String token, String database, String layout, String recordId, vector<RecordField> fields)
{
  String url(stringf(URL_RECORD, database.c_str(), layout.c_str(), recordId.c_str()));
  log_d("Url: %s", url.c_str());
  String payload = generateCreatePayload(fields);
  log_d("Payload: %s", payload.c_str());
  String auth = generateAuth(token.c_str());
  log_d("Authorization: %s", auth.c_str());

  if (!this->_https.begin(
          this->_client,
          this->_host,
          443,
          url, true))
  {
    log_e("Could not connect to: %s", this->_host.c_str());
    return EMPTY_STRING;
  }
  this->_https.setAuthorization(EMPTY_STRING);
  this->_https.setUserAgent(HEADER_AGENT_VALUE);
  this->_https.setReuse(false);
  this->_https.addHeader(HEADER_AUTHORIZATION, auth.c_str());
  this->_https.addHeader(HEADER_ACCEPT, HEADER_ACCEPT_VALUE_ALL);
  this->_https.addHeader(HEADER_CACHE_CONTROL, HEADER_CACHE_CONTROL_VALUE_NO_CACHE);
  this->_https.addHeader(HEADER_CONTENT_TYPE, MIME_TYPE_APPLICATION_JSON);
  int httpCode = this->_https.PATCH(payload);
  const String &response = this->_https.getString();
  log_d("Response: %s", response.c_str());
  this->_https.end();
  if (httpCode == HTTP_CODE_OK)
  {
    log_d("Successfull request - Status: %d", httpCode);
    return response;
  }
  else
  {
    log_e("Http error: %d - %s", httpCode, this->_https.errorToString(httpCode));
  }
  return EMPTY_STRING;
}

String FMDataClient::editRecord(String database, String layout, String recordId, vector<RecordField> fields)
{
  if (this->_token == EMPTY_STRING)
  {
    log_e("Error token is empty");
    return EMPTY_STRING;
  }
  else
  {
    return this->editRecord(this->_token, database, layout, recordId, fields);
  }
}
/**
   * @brief Delete a record
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_delete-record
   * @param token 
   * @param database 
   * @param layout 
   * @param recordId 
   * @return boolean 
   */
boolean FMDataClient::deleteRecord(String token, String database, String layout, String recordId)
{
  String url(stringf(URL_RECORD, database.c_str(), layout.c_str(), recordId.c_str()));
  log_d("Url: %s", url.c_str());
  String payload(EMPTY_STRING);
  log_d("Payload: %s", payload.c_str());
  String auth = generateAuth(token.c_str());
  log_d("Authorization: %s", auth.c_str());

  if (!this->_https.begin(
          this->_client,
          this->_host,
          443,
          url, true))
  {
    log_e("Could not connect to: %s", this->_host.c_str());
    return EMPTY_STRING;
  }
  this->_https.setAuthorization(EMPTY_STRING);
  this->_https.setUserAgent(HEADER_AGENT_VALUE);
  this->_https.setReuse(false);
  this->_https.addHeader(HEADER_AUTHORIZATION, auth.c_str());
  this->_https.addHeader(HEADER_ACCEPT, HEADER_ACCEPT_VALUE_ALL);
  this->_https.addHeader(HEADER_CACHE_CONTROL, HEADER_CACHE_CONTROL_VALUE_NO_CACHE);
  int httpCode = this->_https.sendRequest(HTTP_METHOD_DELETE, payload);
  const String &response = this->_https.getString();
  log_d("Response: %s", response.c_str());
  this->_https.end();
  if (httpCode == HTTP_CODE_OK)
  {
    log_d("Successfull request - Status: %d", httpCode);
    return response;
  }
  else
  {
    log_e("Http error: %d - %s", httpCode, this->_https.errorToString(httpCode));
  }
  return EMPTY_STRING;
}
/**
   * @brief Delete a record
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_delete-record
   * @param database 
   * @param layout 
   * @param recordId 
   * @return boolean 
   */
boolean FMDataClient::deleteRecord(String database, String layout, String recordId)
{
  if (this->_token == EMPTY_STRING)
  {
    log_e("Error token is empty");
    return EMPTY_STRING;
  }
  else
  {
    return this->deleteRecord(this->_token, database, layout, recordId);
  }
}
/**
 * @brief 
 * 
 * @param token 
 * @param database 
 * @param layout 
 * @param recordId 
 * @param ranges 
 * @return String 
 */
String FMDataClient::getRecord(String token, String database, String layout, String recordId, PortalRecordRange *ranges)
{
  throw ERROR_MSG_NOT_IMPLEMENTED;
}

/**
   * @brief Log in to a database session
   *  
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-in
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-in-eds
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-in-oauth
   * @return String Filemaker response
   */
String FMDataClient::logInToDatabaseSession(void)
{
  this->_client.stop();
  if (this->_credentials->getType() == CredentialsType::UserCredentialsType)
  {
    log_d("Starting a new session");
    if (!this->_https.begin(
            this->_client,
            this->_host,
            this->_port,
            this->_credentials->getLogInUrl(),
            true))
    {
      log_e("Could not connect to: %s", this->_host.c_str());
      return EMPTY_STRING;
    }
    String payload(this->_credentials->getExternalDatabasesCredentialsPayload());
    log_d("Payload: %s", payload.c_str());
    size_t size = payload.length();
    log_d("Payload length: %d", size);
    String auth = this->_credentials->getAuthorizationHeaderValue();
    log_d("Database Credentials: %s", auth.c_str());
    this->_https.setUserAgent(HEADER_AGENT_VALUE);
    this->_https.setReuse(false);
    this->_https.setAuthorization(auth.c_str());
    this->_https.addHeader(HEADER_CONTENT_TYPE, MIME_TYPE_APPLICATION_JSON);
    this->_https.addHeader(HEADER_CONTENT_LENGTH, String(size));
    int httpCode = this->_https.POST(payload);
    const String &response = this->_https.getString();
    log_d("Response: %s", response.c_str());
    this->_https.end();
    if (httpCode == HTTP_CODE_OK)
    {
      log_d("Successfull request - Status: %d", httpCode);
      const size_t capacity = // JSON Memory Size
          JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + 191;
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, response);
      if (error)
      {
        log_e("ArduinoJson error - %s", error.c_str());
        return EMPTY_STRING;
      }
      else
      {
        JsonArray messages = doc[PARAMETER_MESSAGES].as<JsonArray>();
        for (JsonObject msg : messages)
        {
          if (msg[PARAMETER_CODE].as<int>() != 0)
          {
            log_e("Filemaker error: %d - %s",
                  msg[PARAMETER_CODE].as<int>(),
                  msg[PARAMETER_MESSAGE].as<const char *>());
            return EMPTY_STRING;
          }
        }
        this->_token = doc[PARAMETER_RESPONSE][PARAMETER_TOKEN].as<String>();
        log_d("Token: %s", this->_token.c_str());
        return response;
      }
    }
    else
    {
      log_e("Http error: %d - %s", httpCode, this->_https.errorToString(httpCode));
    }
  }
  else
  {
    log_e(ERROR_MSG_NOT_IMPLEMENTED);
  }
  return EMPTY_STRING;
}
/**
  * @brief Construct a new FMDataClient object
  * 
  * @param client Wifi Client
  * @param credentials Database Credentials
  * @param host Host address, ip address or domain name
  * @param cert Root Certificate
  * @param port Connection Port
  */
FMDataClient::FMDataClient(
    const WiFiClientSecure &client,
    const DatabaseCredentials &credentials,
    const char *&host,
    const char *&cert,
    const int &port)
{
  this->_client = client;
  this->_credentials = &credentials;
  this->_host = host;
  this->_cert = cert;
  this->_port = port;
  this->_client.setCACert(cert);
};
/**
   * @brief Destroy the FMDataClient object
   * 
   */
FMDataClient::~FMDataClient(void)
{
  delete[] & _https;
  delete[] & _client;
  delete[] _credentials;
}

/**
 * @brief 
 * 
 * @param token 
 * @param database 
 * @param layout 
 * @param range 
 * @return String 
 */
String FMDataClient::getRecords(String token, String database, String layout, RecordRange range)
{
  throw ERROR_MSG_NOT_IMPLEMENTED;
}

/**
 * @brief 
 * 
 * @param token 
 * @param database 
 * @param layout 
 * @param sortCriteria 
 * @param range 
 * @return String 
 */
String FMDataClient::getRecords(String token, String database, String layout, SortCriteria sortCriteria, RecordRange range)
{
  throw ERROR_MSG_NOT_IMPLEMENTED;
}

 /**
   * @brief Upload container data
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#upload-container-data
   * @param token Authentication Token
   * @param database Database Name
   * @param layout Layout Name
   * @param recordId Record Identifier
   * @param fieldName Field Name
   * @param repetition Field Repetition index
   * @return String Json with result response or empty in case of error
   */
String FMDataClient::uploadContainerData(String token, String database, String layout, String recordId, String fieldName, int repetition)
{
  String url(stringf(
      URL_CONTAINER,
      database.c_str(),
      layout.c_str(),
      recordId.c_str(),
      fieldName.c_str(),
      String(repetition).c_str()));
  log_d("Url: %s", url.c_str());
  String payload = EMPTY_STRING;
  log_d("Payload: %s", payload.c_str());
  String auth = generateAuth(this->_token.c_str());
  log_d("Authorization: %s", auth.c_str());

  if (!this->_https.begin(
          this->_client,
          this->_host,
          443,
          url, true))
  {
    log_e("Could not connect to: %s", this->_host.c_str());
    return EMPTY_STRING;
  }
  this->_https.setAuthorization(EMPTY_STRING);
  this->_https.setUserAgent(HEADER_AGENT_VALUE);
  this->_https.setReuse(false);
  this->_https.addHeader(HEADER_AUTHORIZATION, auth.c_str());
  this->_https.addHeader(HEADER_ACCEPT, HEADER_ACCEPT_VALUE_ALL);
  this->_https.addHeader(HEADER_CACHE_CONTROL, HEADER_CACHE_CONTROL_VALUE_NO_CACHE);
  this->_https.addHeader(HEADER_CONTENT_TYPE, MIME_TYPE_MULTIPART_FORM_DATA);
  int httpCode = this->_https.POST(payload);
  const String &response = this->_https.getString();
  log_d("Response: %s", response.c_str());
  this->_https.end();
  if (httpCode == HTTP_CODE_OK)
  {
    log_d("Successfull request - Status: %d", httpCode);
    return response;
  }
  else
  {
    log_e("Http error: %d - %s", httpCode, this->_https.errorToString(httpCode));
  }
  return EMPTY_STRING;
}
/**
   * @brief Upload container data
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#upload-container-data
   * @param database Database Name
   * @param layout Layout Name
   * @param recordId Record Identifier
   * @param fieldName Field Name
   * @param repetition Field Repetition index
   * @return String Json with result response or empty in case of error
   */
String FMDataClient::uploadContainerData(String database, String layout, String recordId, String fieldName, int repetition)
{
  if (this->_token == EMPTY_STRING)
  {
    log_e("Error token is empty");
    return EMPTY_STRING;
  }
  else
  {
    return this->uploadContainerData(this->_token, database, layout, recordId, fieldName, repetition);
  }
}
/**
 * @brief 
 * 
 * @param token 
 * @param database 
 * @param layout 
 * @param findCriteria 
 * @param sortCriteria 
 * @return String 
 */
String FMDataClient::performFind(String token, String database, String layout, FindCriteria *findCriteria, SortCriteria *sortCriteria)
{
  throw ERROR_MSG_NOT_IMPLEMENTED;
}

/**
 * @brief 
 * 
 * @param token 
 * @param database 
 * @param records 
 * @return String 
 */
String FMDataClient::setGlobalVariables(String token, String database, RecordField *records)
{
  throw ERROR_MSG_NOT_IMPLEMENTED;
}

/**
 * @brief Get the Authentication Token
 * 
 * @return String 
 */
String FMDataClient::getToken(void)
{
  return this->_token;
}

/**
 * @brief Format a string
 * Format a string, max length 512
 * @param format format template
 * @param ... fill parameters
 * @return char* formated string
 */
char *stringf(const char *format, ...)
{
  static char buffer[512] = "";

  va_list argptr;
  va_start(argptr, format);

  vsprintf(buffer, format, argptr);

  va_end(argptr);

  return buffer;
}

String FMDataClient::generateCreatePayload(vector<RecordField> fields)
{
  String result = EMPTY_STRING;
  size_t numChars = 0;
  for (auto field : fields)
    numChars += field.getSize();
  size_t numFields = fields.size() + 1;
  size_t size = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(numFields) +
                numFields * 16 + numChars;
  DynamicJsonDocument doc(size);
  JsonObject fieldData = doc.createNestedObject(PARAMETER_FIELD_DATA);
  for (auto field : fields)
  {
    log_d("Field Name: %s", field.fieldName.c_str());
    log_d("Field Value: %s", field.fieldValue.c_str());
    if (field.fieldType == FieldTypes::Number)
    {
      log_d("Field Value is Number");
      fieldData[field.fieldName] = atoi(field.fieldValue.c_str());
    }
    else
    {
      log_d("Field Value is Text");
      fieldData[field.fieldName] = field.fieldValue;
    }
  }
  log_d("Create Record payload size: %d", measureJson(doc));
  serializeJson(doc, result);
  return result;
}