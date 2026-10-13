#pragma once

#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <android/log.h>
#include <android/input.h>
#include <android_native_app_glue.h>
#include <android/native_activity.h>

#define LOG_TAG "MA_APP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)



namespace MA {
struct CredentialRecord {
  std::string password;
  std::string controll;
  bool found = false;
};

class Database {
// ==========================
// === Data =================
// ==========================
private:
  std::string db_path = "/data/data/com.ma.app/databases/vault.db";
  std::string controll_value = "ashuda7y8s6d75tadahba89dasdjjbzkhd";

// ==========================
// === Functions ============
// ==========================
// === Constructors
public:
  Database();
  ~Database();
  Database(const Database& second) = delete;
  Database& operator=(const Database& second) = delete;
  Database(Database&& second) = delete;
  Database& operator=(Database&& second) = delete;

// === Database Events
public:
  std::string getPassword(const std::string& db_password, const std::string& name);
  int storePassword(const std::string& db_password, const std::string& name, const std::string& password);
  int removePassword(const std::string& name);

// === Hellpers
  std::string pbkdf2_hashing(const std::string& db_password);
  
  bool initializeDatabase();
  bool saveCredential(const std::string& name, const std::string& password, const std::string& controll);
  MA::CredentialRecord getCredential(const std::string& name); 
  int deleteCredential(const std::string& name);

  std::vector<uint8_t> str_to_bytes(const std::string& data);
  std::string bytes_to_str(const std::vector<uint8_t>& bytes);
  std::vector<uint8_t> aes_encrypt(const std::vector<uint8_t>& key_bytes, const std::vector<uint8_t>& data_bytes);
  std::vector<uint8_t> aes_decrypt(const std::vector<uint8_t>& key_bytes, const std::vector<uint8_t>& data_bytes);

};
};

