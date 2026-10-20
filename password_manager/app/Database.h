// Password Manager
// Copyright 2026 Daynlight
// Licensed under the GNU General, Version 3.0.
// See LICENSE file for details.



#pragma once
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <filesystem>

#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <android/log.h>
#include <android/input.h>
#include <android_native_app_glue.h>
#include <android/native_activity.h>

#define LOG_TAG "Password_Manager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include "Config.h"



namespace MA {
struct CredentialRecord {
  std::string password;
  std::string controll;
  bool found = false;
};



class Database {
// ==============================================
// === Data =====================================
// ==============================================
private:
  const std::filesystem::path db_path = MA::Config::DB_FILE_PATH; 

// ==============================================
// === Functions ================================
// ==============================================
// ==================== //
// === Constructors === //
// ==================== //
public:
  Database();
  ~Database();
  Database(const Database& second) = delete;
  Database& operator=(const Database& second) = delete;
  Database(Database&& second) = delete;
  Database& operator=(Database&& second) = delete;

// ======================= //
// === Database Events === //
// ======================= //
public:
  std::string getPassword(const std::string& db_password, const std::string& name);
  int storePassword(const std::string& db_password, const std::string& name, const std::string& password);
  int removePassword(const std::string& name);
  bool loginToPasswordManager(const std::string& password); 
  int addDatabaseAccessPassword(const std::string& password, const std::string& password_to_add);
  int removeDatabaseAccessPassword(const std::string& password, const std::string password_to_remove);

// ======================= //
// === Helpers =========== //
// ======================= //
private:
// === Crypt
  std::string pbkdf2_hashing(const std::string& db_password);
  std::vector<uint8_t> aes_encrypt(const std::vector<uint8_t>& key_bytes, const std::vector<uint8_t>& data_bytes);
  std::vector<uint8_t> aes_decrypt(const std::vector<uint8_t>& key_bytes, const std::vector<uint8_t>& data_bytes);

// === Database Querry
  bool initializeDatabaseQuerry();
  bool saveCredentialQuerry(const std::string& name, const std::string& password, const std::string& controll);
  MA::CredentialRecord getCredentialQuerry(const std::string& name); 
  int deleteCredentialQuerry(const std::string& name);
  bool accessToDatabaseQuerry(const std::string& hashed_password);
  int addAccessToDatabaseQuerry(const std::string& hashed_password, const std::string& hashed_password_to_add);
  int removeAccessToDatabaseQuerry(const std::string& hashed_password, const std::string& hashed_password_to_remove);

// === Casting
  std::vector<uint8_t> str_to_bytes(const std::string& data);
  std::string bytes_to_str(const std::vector<uint8_t>& bytes);
};
};

