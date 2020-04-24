/*
  FMDataClient.h - Filemaker DATA API library 
  Copyright (c) 2020 Bruno Silva.  DrM, Dr. Mueller AG.
*/

// ensure this library description is only included once
#ifndef FMDataClient_h
#define FMDataClient_h

// include types & constants of Wiring core API
#include <stdio.h>
#include <stdarg.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>

#define EMPTY_STRING ""

#define HEADER_X_FM_DATA_ACCESS_TOKEN "X-FM-Data-Access-Token"
#define HEADER_X_FMS_REQUEST_ID "X-FMS-Request-ID"
#define HEADER_CONTENT_TYPE "Content-Type"
#define HEADER_AUTHORIZATION "Authorization"
#define HEADER_X_FM_DATA_OAUTH_REQUEST_ID "X-FM-Data-OAuth-Request-Id"
#define HEADER_X_FM_DATA_OAUTH_IDENTIFIER "X-FM-Data-OAuth-Identifier"
#define HEADER_AGENT_VALUE "DrM IoT Device/1.1 (ESP32; Arduino 1.8.12)"
#define HEADER_CONNECTION "Connection"
#define HEADER_CONNECTION_CLOSE "close"
#define HEADER_CONNECTION_KEEP_ALIVE "keep-alive"
#define HEADER_HOST "Host"
#define HEADER_ACCEPT "Accept"
#define HEADER_ACCEPT_VALUE_ALL "*/*"
#define HEADER_CACHE_CONTROL "Cache-Control"
#define HEADER_CACHE_CONTROL_VALUE_NO_CACHE "no-cache"
#define HEADER_CONTENT_LENGTH "Content-Length"
#define HEADER_CONTENT_LENGTH_EMPTY "0"

#define HTTP_METHOD_DELETE "DELETE"
#define HTTP_BOUNDARY
#define MIME_TYPE_APPLICATION_JSON "application/json; charset=utf-8"
#define MIME_TYPE_MULTIPART_FORM_DATA "multipart/form-data; boundary="

#define URL_FULL "https://%s:%d%s"
#define URL_DATA_API_BASE_V1 "/fmi/data/v1/databases/"
#define URL_SESSIONS "/fmi/data/v1/databases/%s/sessions"
#define URL_OAUTH_PROVIDERS "/fmws/oauthproviderinfo"
#define URL_OAUTH_REQUEST_ID "/oauth/getoauthurl?trackingID=%s&provider=%s&address=%s&X-FMS-OAuth-AuthType=%s"
#define URL_SESSION_DELETE "/fmi/data/v1/databases/%s/sessions/%s"
#define URL_RECORD_NEW "/fmi/data/v1/databases/%s/layouts/%s/records"
#define URL_RECORD "/fmi/data/v1/databases/%s/layouts/%s/records/%s"
#define URL_RECORDS "/fmi/data/v1/databases/%s/layouts/%s/records"
#define URL_CONTAINER "/fmi/data/v1/databases/%s/layouts/%s/records/%s/containers/%s/%s"
#define URL_FIND "/fmi/data/v1/databases/%s/layouts/%s/_find"
#define URL_GLOBALS "/fmi/data/v1/databases/%s/globals"

#define PARAMETER_RESPONSE "response"
#define PARAMETER_MESSAGES "messages"
#define PARAMETER_MESSAGE "message"
#define PARAMETER_CODE "code"
#define PARAMETER_DATA "data"
#define PARAMETER_FM_DATA_SOURCE "fmDataSource"
#define PARAMETER_DATABASE "database"
#define PARAMETER_USER_NAME "username"
#define PARAMETER_PASSWORD "password"
#define PARAMETER_OAUTH_REQUEST_ID "oAuthRequestId"
#define PARAMETER_OAUTH_IDENTIFIER "oAuthIdentifier"
#define PARAMETER_FIELD_DATA "fieldData"
#define PARAMETER_PORTAL_DATA "portalData"
#define PARAMETER_DELETE_RELATED "deleteRelated"
#define PARAMETER_OFFSET "_offser"
#define PARAMETER_LIMIT "_limit"
#define PARAMETER_PORTAL "portal"
#define PARAMETER_QUERY "query"
#define PARAMETER_SORT "sort"
#define PARAMETER_GLOBAL_FIELD "globalFields"
#define PARAMETER_SCRIPT_NAME "sript"
#define PARAMETER_SCRIPT_PARAMETER "sript.param"
#define PARAMETER_SCRIPT_PRE_REQUEST_NAME "sript.prerequest"
#define PARAMETER_SCRIPT_PRE_REQUEST_PARAMETER "sript.prerequest.param"
#define PARAMETER_SCRIPT_PRE_SORT_NAME "sript.presort"
#define PARAMETER_SCRIPT_PRE_SORT_PARAMETER "sript.presort.param"
#define PARAMETER_TOKEN "token"
#define PARAMETER_BEARER "Bearer "

#define ERROR_MSG_EMPTY_DATABASE_NAME "Empty Database Name"
#define ERROR_MSG_EMPTY_USER_NAME "Empty User Name"
#define ERROR_MSG_EMPTY_PASSWORD "Empty Password"
#define ERROR_MSG_EMPTY_OAUTH_REQUEST_ID "Empty OAuth Request Id"
#define ERROR_MSG_EMPTY_OAUTH_ID "Empty OAuth Id"
#define ERROR_MSG_EMPTY_CRENDENTIALS "No database credentials"
#define ERROR_MSG_NOT_IMPLEMENTED "Not Implemented"
#define ERROR_MSG_UNKNOWN_ERROR "Unknown Error"
#define ERROR_MSG_EMPTY_TOKEN "Empty Token"
#define ERROR_MSG_CONNECTION_FAILED "Connection Failed"

// #define setf(format, ...) formatString(format, ##__VA_ARGS__)

using namespace std;

char *stringf(const char *format, ...);

/**
 * @brief Credentials Type
 * 
 */
enum CredentialsType
{
  UserCredentialsType,
  OAuthUserCredentialsType
};

/**
 * @brief Database Credentials
 * 
 */
class DatabaseCredentials
{
public:
  DatabaseCredentials(String database);
  DatabaseCredentials(
      String database,
      const vector<DatabaseCredentials *> &externalDatabasesCredentials);
  String getExternalDatabasesCredentialsPayload(void) const;
  virtual CredentialsType getType(void) const = 0;
  virtual String getAuthorizationHeaderValue(void) const = 0;
  String getLogInUrl(void) const;
  String getLogOutUrl(const String &token) const;

protected:
  String database;
  virtual JsonObject toJSON(void) = 0;
  vector<DatabaseCredentials *> externalDatabasesCredentials;
};
/**
 * @brief User Databaser Credentials
 * 
 */
class UserCredentials : public DatabaseCredentials
{
public:
  UserCredentials(String database, String userName, String password);
  UserCredentials(
      String database,
      String userName,
      String password,
      const vector<DatabaseCredentials *> &externalDatabasesCredentials);
  String getAuthorizationHeaderValue(void) const;
  CredentialsType getType(void) const;

protected:
  JsonObject toJSON(void);
  String userName;
  String password;
};
/**
 * @brief OAuth Database Credentials
 * 
 */
class OAuthUserCredentials : public DatabaseCredentials
{
public:
  OAuthUserCredentials(String database, String oAuthRequestId, String oAuthId);
  OAuthUserCredentials(
      String database,
      String oAuthRequestId,
      String oAuthId,
      const vector<DatabaseCredentials *> &externalDatabasesCredentials);
  String getAuthorizationHeaderValue(void) const;
  CredentialsType getType(void) const;

protected:
  JsonObject toJSON(void);
  String oAuthRequestId;
  String oAuthId;
};

class PortalRecordRange
{
public:
  PortalRecordRange(String portalName, int offset = 0, int limit = 50);
  String PortalName;
  int Offset;
  int Limit;
};

class RecordRange
{
public:
  RecordRange(int offset = 0, int limit = 100);
  RecordRange(const vector<PortalRecordRange> &portalRanges, int offset = 0, int limit = 100);
  PortalRecordRange *PortalRanges;
  int Offset;
  int Limit;
  String getQueryString(void) const;
};

enum SortOrder
{
  ascend,
  descend
};

enum FieldTypes
{
  Text,
  Number,
  Date,
  Time,
  Timestamp,
  Container,
  Function,
  Summary
};

class SortCriteria
{
public:
  SortCriteria(String fieldName, SortOrder order = SortOrder::ascend);
  SortOrder Order;
  String FieldName;
  String toJSON(void) const;
};

class FindCriteria
{
public:
  FindCriteria(String fieldName, String value, boolean omit = false);
  String FieldName;
  String Value;
  boolean Omit;
  String toJSON(void) const;
};

class RecordField
{
public:
  RecordField(String fieldName, String fieldValue = "", FieldTypes fieldType = FieldTypes::Text);
  RecordField(String fieldName, int fieldValue);
  RecordField(String fieldName, float fieldValue);
  String fieldName;
  String fieldValue;
  FieldTypes fieldType;
  String toJSON(void) const;
  size_t getSize();
};

class ScriptParameters
{
public:
  ScriptParameters(String name = "", String parameter = "",
                   String preRequestScriptName = "", String preRequestScriptParameter = "",
                   String preSortScriptName = "", String preSortScriptParameter = "");
  String toJSON(void) const;
  String toQueryString(void) const;
};

/**
 * @brief Filemaker DATA API Client
 * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/
 */
class FMDataClient
{
public:
  /**
   * @brief Construct a new FMDataClient object
   * 
   * @param client Wifi Client
   * @param credentials Database Credentials
   * @param host Host address, ip address or domain name
   * @param cert Root Certificate
   * @param port Connection Port
   */
  FMDataClient(
      const WiFiClientSecure &client,
      const DatabaseCredentials &credentials,
      const char *&host,
      const char *&cert,
      const int &port);
  /**
   * @brief Destroy the FMDataClient object
   * 
   */
  ~FMDataClient();

  /**
   * @brief Log in to a database session
   *  
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-in
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-in-eds
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-in-oauth
   * @return String Filemaker response
   */
  String logInToDatabaseSession(void);

  /**
   * @brief Get the list of supported OAuth providers and tracking Id
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-in-oauth
   * @return String Filemaker response
   */
  String getListOfOAuthProviders(void);

  /**
   * @brief Get the OAuth request ID and Authentication URL
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-in-oauth
   * @param trackingId Tracking Id
   * @param oauthProvider OAuth Provider
   * @param address Callback address
   * @param oauthType OAuth Type
   * @return String 
   */
  String getOAuthRequestId(String trackingId, String oauthProvider, String address = "127.0.0.1", int oauthType = 2);

  /**
   * @brief Log out of a database session
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database_log-out
   * @return String Filemaker response
   */
  String logOutDatabaseSession(void);

  /**
   * @brief Create a record
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_create-record
   * @param token The Authentication Token
   * @param database Database Name
   * @param layout Layout Name
   * @param fields List of fields with values
   * @return String Json with result or empty string when it fails
   */
  String createRecord(String token, String database, String layout, vector<RecordField> fields);
  /**
   * @brief Create a Record object
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_create-record
   * @param database Database Name
   * @param layout Layout Name
   * @param fields List of fields with values
   * @return String Json with result or empty string when it fails
   */
  String createRecord(String database, String layout, vector<RecordField> fields);

  /**
   * @brief Edit a record
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_edit-record
   * @param token The Authentication Token
   * @param database Database Name
   * @param layout Layout Name
   * @param recordId  Record Identifier
   * @param fields List of fields with values
   * @return String Json with result or empty string when it fails
   */
  String editRecord(String token, String database, String layout, String recordId, vector<RecordField> fields);
  /**
   * @brief Edit a record
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_edit-record
   * @param database Database Name
   * @param layout Layout Name
   * @param recordId  Record Identifier
   * @param fields List of fields with values
   * @return String Json with result or empty string when it fails
   */
  String editRecord(String database, String layout, String recordId, vector<RecordField> fields);

  /**
   * @brief Delete a record
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_delete-record
   * @param token 
   * @param database 
   * @param layout 
   * @param recordId 
   * @return boolean 
   */
  boolean deleteRecord(String token, String database, String layout, String recordId);
  /**
   * @brief Delete a record
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_delete-record
   * @param database 
   * @param layout 
   * @param recordId 
   * @return boolean 
   */
  boolean deleteRecord(String database, String layout, String recordId);

  /**
   * @brief Get a single record
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_get-record
   * @param token 
   * @param database 
   * @param layout 
   * @param recordId 
   * @param ranges 
   * @return String 
   */
  String getRecord(String token, String database, String layout, String recordId, PortalRecordRange *ranges = NULL);

  /**
   * @brief Get a range of records
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_get-records
   * @param token 
   * @param database 
   * @param layout 
   * @param range 
   * @return String 
   */
  String getRecords(String token, String database, String layout, RecordRange range = RecordRange());

  /**
   * @brief Get a range of records
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_get-records
   * @param token 
   * @param database 
   * @param layout 
   * @param sortCriteria 
   * @param range 
   * @return String 
   */
  String getRecords(String token, String database, String layout, SortCriteria sortCriteria, RecordRange range = RecordRange());

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
  String uploadContainerData(String token, String database, String layout, String recordId, String fieldName, int repetition = 1);

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
  String uploadContainerData(String database, String layout, String recordId, String fieldName, int repetition = 1);

  /**
   * @brief Perform a find request
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#perform-a-find-request
   * @param token 
   * @param database 
   * @param layout 
   * @param findCriteria 
   * @param sortCriteria 
   * @return String 
   */
  String performFind(String token, String database, String layout, FindCriteria *findCriteria, SortCriteria *sortCriteria);

  /**
   * @brief Set global field values
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#set-global-fields
   * @param token 
   * @param database 
   * @param records 
   * @return String 
   */
  String setGlobalVariables(String token, String database, RecordField *records);

  /**
   * @brief Get the Authentication
   * 
   * @return String 
   */
  String getToken(void);

private:
  String _cert;
  WiFiClientSecure _client;
  HTTPClient _https;
  String _host;
  int _port;
  const DatabaseCredentials *_credentials;
  /**
   * @brief Authentication Token
   * The access token must be used in the header of all subsequent calls to the hosted database. 
   * The access token is valid until you log out of a database session or for 15 minutes 
   * after the last call that specified the token. (While the token is valid, each call that 
   * specifies the token resets the session timeout counter to zero.)
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#connect-database
   */
  String _token;

  /**
   * @brief Generated the payload to create new record
   * 
   * @param fieds List of fields
   * @return String Filemaker response
   */
  static String generateCreatePayload(vector<RecordField> fields);

  /**
   * @brief Generate Bearer token authorization
   * 
   * @param token  Token
   * @return String 
  */
  static String generateAuth(const char *token);
};

#endif