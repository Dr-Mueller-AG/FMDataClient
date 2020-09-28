# Filemaker 17 DATA API Client ESP32

Filemaker Data Api client for Expressif32 boards

[![Build Status](https://travis-ci.org/bmts/FMDataClient.svg?branch=master)](https://travis-ci.org/bmts/FMDataClient)

---

## Features

- :arrow_right: Login
  - :+1: User Database Credentials
  - :+1: External User Database Credentials
  - :x: User OAuth Credentials
  - :x: Token Management
- :+1: Logout
- :+1: Create Record
- :x: Edit Record
- :x: Delete Record
- :x: Get Record
- :x: Get records
- :x: Upload Container
- :+1: Find Records
- :x: Set Global Variables
- :arrow_right: Preform Scripts by parameter
  - :+1: Create Record
  - :x: Edit Record
  - :x: Delete Record
  - :x: Get Record
  - :x: Get Records
  - :x: Upload Container
  - :+1: Find Records
  - :x: Set Global Variables ::

---

## Sample Code

```c++
    WiFiClientSecure wifi;
    UserCredentials credentials(database, userName, password);
    FMDataClient client(wifi, credentials, host, cert, port);
    ...
    client.logInToDatabaseSession();
    ...
    vector<RecordField> recordFields;
    RecordField field1("field1", "data1", FieldTypes::Text);
    RecordField field2("field2", "data2", FieldTypes::Text);
    recordFields.push_back(field1);
    recordFields.push_back(field2);
    client.createRecord(client.getToken(), database, layout, recordFields);
    ...
    client.logOutDatabaseSession();
```

## References

[FileMaker 17 Data API Guide](https://fmhelp.filemaker.com/docs/17/en/dataapi/)
[FileMaker Pro error code reference guide](https://support.claris.com/s/article/FileMaker-Pro-error-code-reference-guide-1503693005688?language=en_US)

---

<div>
  <img style="border:1px solid #bababa; padding: 6px;" src="https://avatars2.githubusercontent.com/u/3626749?s=150"/>
</div>

__Bruno Silva__
:mailbox: bruno.silva@drm.ch
:mailbox: bruno_m_silva@sapo.pt