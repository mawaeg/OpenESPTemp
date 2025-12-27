#include "CredentialManager.h"


CredentialManager::CredentialManager(){}
CredentialManager::~CredentialManager(){}

void CredentialManager::put(const char* key, const char* value){
    preferences.begin("credentials", false);
    preferences.putString(key, value);
    preferences.end();
}

void CredentialManager::put(CredentialType key, const char* value){
    this->put(credential_keys[key], value);
}

String CredentialManager::get(const char* key){
    preferences.begin("credentials", false);
    String value = String(preferences.getString(key, "").c_str());
    preferences.end();
    return value;
}

String CredentialManager::get(CredentialType key){
    return this->get(credential_keys[key]);
}

void CredentialManager::remove(const char* key){
    preferences.begin("credentials", false);
    preferences.remove(key);
    preferences.end();
}

void CredentialManager::remove(CredentialType key){
    this->remove(credential_keys [key]);
}

bool CredentialManager::is_valid(){
    bool valid = true;
    for (int i = 0; i < credential_keys.size(); i++){
        if (this->get(credential_keys[i]) == ""){
            valid = false;
            break;
        }
    }
    return valid;
}


