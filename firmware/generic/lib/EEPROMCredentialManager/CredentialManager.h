#ifndef CREDENTIALMANAGER_H
#define CREDENTIALMANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <string>
#include <vector>

enum CredentialType{
  SSID,
  PASSWORD,
  POST_URL,
  AUTHORIZATION
};

const std::vector<const char*> credential_keys = {"ssid", "password", "post_url", "authorization"};

class CredentialManager {
  public:
    CredentialManager();
    ~CredentialManager();

    void put(const char* key, const char* value);
    void put(CredentialType key, const char* value);
    String get(const char* key);
    String get(CredentialType key);
    void remove(const char* key);
    void remove(CredentialType key);
    bool is_valid();
    
  private:
    Preferences preferences;  
};

#endif /* CREDENTIALMANAGER_H */