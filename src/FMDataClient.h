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
#include <StreamString.h>

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
#define HEADER_CONTENT_DISPOSITION "Content-Disposition"
#define HEADER_GENERIC "%s: %s"

#define FORM_DATA_DISPOSITION "form-data; name=\"upload\"; filename=\"%s\""

#define HTTP_METHOD_DELETE "DELETE"
#define HTTP_METHOD_POST "POST"
#define HTTP_METHOD_GET "POST"
#define HTTP_METHOD_PUT "PUT"
#define HTTP_METHOD_PATCH "PATCH"
#define HTTP_BOUNDARY "----WebKitFormBoundary7MA4YWxkTrZu0gW"

#define MIME_TYPE_APPLICATION_JSON "application/json; charset=utf-8"
#define MIME_TYPE_MULTIPART_FORM_DATA "multipart/form-data; boundary="
#define MIME_TYPE_TEXT "text/plain"
#define MIME_TYPE_CSV "text/csv"

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
#define PARAMETER_FIELD_NAME "fieldName"
#define PARAMETER_PORTAL_DATA "portalData"
#define PARAMETER_DELETE_RELATED "deleteRelated"
#define PARAMETER_OFFSET "offset"
#define PARAMETER_LIMIT "limit"
#define PARAMETER_PORTAL "portal"
#define PARAMETER_QUERY "query"
#define PARAMETER_SORT "sort"
#define PARAMETER_SORT_ASCEND "ascend"
#define PARAMETER_SORT_DESCEND "descend"
#define PARAMETER_SORT_ORDER "sortOrder"
#define PARAMETER_GLOBAL_FIELD "globalFields"
#define PARAMETER_SCRIPT_NAME "script"
#define PARAMETER_SCRIPT_PARAMETER "script.param"
#define PARAMETER_SCRIPT_PRE_REQUEST_NAME "script.prerequest"
#define PARAMETER_SCRIPT_PRE_REQUEST_PARAMETER "script.prerequest.param"
#define PARAMETER_SCRIPT_PRE_SORT_NAME "script.presort"
#define PARAMETER_SCRIPT_PRE_SORT_PARAMETER "script.presort.param"
#define PARAMETER_TOKEN "token"
#define PARAMETER_BEARER "Bearer "
#define PARAMETER_OMIT "omit"
#define PARAMETER_OMIT_TRUE "true"
#define PARAMETER_OMIT_FALSE "false"

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

class SortCriteriaField
{
public:
  SortCriteriaField(String fieldName, SortOrder order = SortOrder::ascend);
  SortOrder order;
  String fieldName;
  JsonObject toJSON(void);
};

class FindCriteriaField
{
public:
  FindCriteriaField(String fieldName, String fieldValue = "*");
  String fieldName;
  String fieldValue;
  JsonObject toJSON(void);
};

class SortCriteria
{
public:
  SortCriteria(const vector<SortCriteriaField *> &records);
  vector<SortCriteriaField *> fields;
  JsonObject toJSON(void);
};

class FindCriteria
{
public:
  FindCriteria(const vector<FindCriteriaField *> &records, boolean omit = false);
  vector<FindCriteriaField *> fields;
  boolean omit;
  JsonObject toJSON(void);
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
  JsonObject toJSON(void) const;
  size_t getSize();
};

/**
 * @brief A class to store, validate and generate the script parameters acording to the request
 * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#running-scripts
 */
class ScriptParameters
{
public:
  /**
   * @brief Construct a new Script Parameters object
   * 
   * @param name Script name
   * @param parameter Script parameters
   * @param preRequestScriptName Prerequest script name
   * @param preRequestScriptParameter Prerequest script parameters
   * @param preSortScriptName Presort script name
   * @param preSortScriptParameter Presort script parameter
   */
  ScriptParameters(String name = "", String parameter = "",
                   String preRequestScriptName = "", String preRequestScriptParameter = "",
                   String preSortScriptName = "", String preSortScriptParameter = "");

  /**
  * @brief Generates de script parameters for POST and PATCH requests
  * 
  * @return JsonObject 
  */
  JsonObject toJSON(void) const;

  /**
   * @brief Generates de script parameters for POST and PATCH requests
   * 
   * @return String 
   */
  String toJSONString(void) const;
  /**
   * @brief Generates de script parameters for GET and DELETE requests
   * 
   * @return String 
   */
  String toQueryString(void) const;

  /**
   * @brief Formats the call prameters, either a Http Query String or a Json String.
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#running-scripts
   * 
   * @param method Http Method
   * @return String The resulting string
   */
  String formatParmaters(String method) const;

private:
  String _name;
  String _parameter;
  String _preRequestScriptName;
  String _preRequestScriptParameter;
  String _preSortScriptName;
  String _preSortScriptParameter;
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
   * @brief Combines two jsons, if key already exists it will be overrided
   * @see https://arduinojson.org/v6/how-to/merge-json-objects/
   * 
   * @param dst destination json
   * @param src source json
   */
  static void mergeJson(JsonVariant dst, JsonVariantConst src);

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
   * @param scripts Scripts to be executed
   * @return String Json with result or empty string when it fails
   */
  String createRecord(String token, String database, String layout, vector<RecordField> fields, ScriptParameters *scripts = NULL);
  /**
   * @brief Create a Record object
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#work-with-records_create-record
   * @param database Database Name
   * @param layout Layout Name
   * @param fields List of fields with values
   * @param scripts Scripts to be executed
   * @return String Json with result or empty string when it fails
   */
  String createRecord(String database, String layout, vector<RecordField> fields, ScriptParameters *scripts = NULL);

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
   * @param scripts
   * @return String 
   */
  String getRecord(String token, String database, String layout, String recordId, PortalRecordRange *ranges = NULL, ScriptParameters *scripts = NULL);

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
   * @param contents Stream
   * @param name Name
   * @param type Content type
   * @return String Json with result response or empty in case of error
   */
  String uploadContainerData(String token, String database, String layout, String recordId, String fieldName, int repetition, String contents, String name, String type);

  /**
   * @brief Upload container data
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#upload-container-data
   * @param database Database Name
   * @param layout Layout Name
   * @param recordId Record Identifier
   * @param fieldName Field Name
   * @param repetition Field Repetition index
   * @param contents Stream
   * @param name Name
   * @param type Content type
   * @return String Json with result response or empty in case of error
   */
  String uploadContainerData(String database, String layout, String recordId, String fieldName, int repetition, String contents, String name, String type);

  /**
   * @brief Perform a find request
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#perform-a-find-request
   * @param database 
   * @param layout 
   * @param findCriterias
   * @param limit
   * @param offset
   * @param sortCriteria 
   * @param scripts
   * @return String 
   */
  String performFind(String database, String layout, vector<FindCriteria *> findCriterias, int limit = 100, int offset = 0, SortCriteria *sortCriteria = NULL, ScriptParameters *scripts = NULL);

  /**
   * @brief Perform a find request
   * @see https://fmhelp.filemaker.com/docs/17/en/dataapi/#perform-a-find-request
   * @param token 
   * @param database 
   * @param layout 
   * @param findCriterias
   * @param limit
   * @param offset
   * @param sortCriteria 
   * @param scripts
   * @return String 
   */
  String performFind(String token, String database, String layout, vector<FindCriteria *> findCriterias, int limit = 100, int offset = 0, SortCriteria *sortCriteria = NULL, ScriptParameters *scripts = NULL);

  /**
   * @brief Generates the find request payload, search criteria, sort criteria and script execution parameters
   * @see performFind()
   * @param findCriteria
   * @param limit
   * @param offset
   * @param sortCriteria 
   * @param scripts
   * @return String 
   */
  String generateFindPayload(vector<FindCriteria *> findCriterias, int limit = 100, int offset = 0, SortCriteria *sortCriteria = NULL, ScriptParameters *scripts = NULL);

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
  String _boundaryString;
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
   * @param fields List of fields
   * @param scripts Scripts to be executed
   * @return String Filemaker response
   */
  static String generatePayload(vector<RecordField> fields, ScriptParameters *scripts = NULL);

  /**
   * @brief Generate Bearer token authorization
   * 
   * @param token  Token
   * @return String 
  */
  static String generateAuth(const char *token);
};

#endif