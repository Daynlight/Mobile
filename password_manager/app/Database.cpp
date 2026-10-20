#include "Database.h"

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif



MA::Database::Database() {
  LOGI("MA::Database Constructor Called");
  initializeDatabase();
};

MA::Database::~Database() {
  LOGI("MA::Database Destructor called");
};


// === Database Events
std::string MA::Database::getPassword(const std::string& db_password, const std::string& name) {
  std::string hashed_password = pbkdf2_hashing(db_password);
  std::vector<uint8_t> hashed_password_bytes = str_to_bytes(hashed_password);    
  
  std::vector<uint8_t> controll_bytes(controll_value.begin(), controll_value.end());
  
  MA::CredentialRecord record = getCredential(name);
  if (!record.found) return "Record didn't exists";

  std::vector<uint8_t> record_controll_bytes = str_to_bytes(record.controll);
  std::vector<uint8_t> record_controll_decrypted = aes_decrypt(hashed_password_bytes, record_controll_bytes);

  if (record_controll_decrypted != controll_bytes) return "Password is invalid don't passed controll block";

  std::vector<uint8_t> record_password_bytes = str_to_bytes(record.password);
  std::vector<uint8_t> record_password_decrypted = aes_decrypt(hashed_password_bytes, record_password_bytes);
  
  return std::string(record_password_decrypted.begin(), record_password_decrypted.end());
};

int MA::Database::storePassword(const std::string& db_password, const std::string& name, const std::string& password) {
  std::string hashed_password = pbkdf2_hashing(db_password);
  std::vector<uint8_t> hashed_password_bytes = str_to_bytes(hashed_password);    
  
  std::vector<uint8_t> controll_bytes(controll_value.begin(), controll_value.end());

  std::vector<uint8_t> controll_encrypted = aes_encrypt(hashed_password_bytes, controll_bytes);
  
  std::vector<uint8_t> password_bytes(password.begin(), password.end());
  std::vector<uint8_t> password_encrypted = aes_encrypt(hashed_password_bytes, password_bytes);
 
  std::string controll_str_encrypted = bytes_to_str(controll_encrypted);
  std::string password_str_encrypted = bytes_to_str(password_encrypted);

  if (controll_str_encrypted.empty() || password_str_encrypted.empty()) {
    LOGE("Encryption failed during storePassword.");
    return -2;
  };

  return saveCredential(name, password_str_encrypted, controll_str_encrypted) ? 0 : -3;
};

int MA::Database::removePassword(const std::string& name) {
  return deleteCredential(name);
};



// === Hellpers
std::string MA::Database::pbkdf2_hashing(const std::string& db_password) {
  std::string salt = "akjskdasjdaksdbzhxcbzuaiuid89u7as89d";
  int iterations = 100000;
  int key_length = 32;

  std::vector<uint8_t> out(key_length);

  int success = PKCS5_PBKDF2_HMAC(
    db_password.data(),
    static_cast<int>(db_password.size()),
    reinterpret_cast<const unsigned char*>(salt.data()),
    static_cast<int>(salt.size()),
    iterations,
    EVP_sha256(),
    key_length,
    out.data()
  );

  if (!success) throw std::runtime_error("OpenSSL PKCS5_PBKDF2_HMAC failed.");

  std::ostringstream oss;
  for (uint8_t b : out) oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
  
  return oss.str();
};



bool MA::Database::initializeDatabase() {
  std::string dir_path = db_path.substr(0, db_path.find_last_of('/'));
  mkdir(dir_path.c_str(), 0700);

  sqlite3* db = nullptr;

  if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
    LOGE("Failed to open/create database: %s", sqlite3_errmsg(db));
    if (db) sqlite3_close(db);
    return false;
  };

  {
    const char* sql = "CREATE TABLE IF NOT EXISTS credentials ("
                      "name TEXT PRIMARY KEY, "
                      "password TEXT NOT NULL, "
                      "controll TEXT NOT NULL);";

    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
      LOGE("SQL credentials  error during table creation: %s", err_msg);
      sqlite3_free(err_msg);
      sqlite3_close(db);
      return false;
    };
  }

  {
    const char* sql = "CREATE TABLE IF NOT EXISTS access ("
                      "id INT PRIMARY KEY, "
                      "hashed_password TEXT NOT NULL);";

    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
      LOGE("SQL access error during table creation: %s", err_msg);
      sqlite3_free(err_msg);
      sqlite3_close(db);
      return false;
    };
  }

  {
    std::string hashed_password = pbkdf2_hashing("admin");

    std::string sql = "INSERT OR REPLACE INTO access (hashed_password) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
      sqlite3_close(db);
      return false;
    };

    sqlite3_bind_text(stmt, 1, hashed_password.c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
  }

  LOGI("Database initialized and 'credentials' table ready.");
  sqlite3_close(db);
  return true;
};

bool MA::Database::saveCredential(const std::string& name, const std::string& password, const std::string& controll) {
  sqlite3* db = nullptr;

  if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
    if (db) sqlite3_close(db);
    return false;
  };

  std::string sql = "INSERT OR REPLACE INTO credentials (name, password, controll) VALUES (?, ?, ?);";
  sqlite3_stmt* stmt = nullptr;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return false;
  };

  sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, controll.c_str(), -1, SQLITE_STATIC);

  bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    
  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return success;
};

MA::CredentialRecord MA::Database::getCredential(const std::string& name) {
  sqlite3* db = nullptr;
  MA::CredentialRecord record;

  if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
    if (db) sqlite3_close(db);
    return record;
  };

  std::string sql = "SELECT password, controll FROM credentials WHERE name = ?;";
  sqlite3_stmt* stmt = nullptr;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return record;
  };

  sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* pwd = sqlite3_column_text(stmt, 0);
    const unsigned char* ctrl = sqlite3_column_text(stmt, 1);

    if (pwd) record.password = reinterpret_cast<const char*>(pwd);
    if (ctrl) record.controll = reinterpret_cast<const char*>(ctrl);
    record.found = true;
  };

  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return record;
};

int MA::Database::deleteCredential(const std::string& name) {
  sqlite3* db = nullptr;

  if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
    if (db) sqlite3_close(db);
    return -2;
  };

  std::string sql = "DELETE FROM credentials WHERE name = ?;";
  sqlite3_stmt* stmt = nullptr;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return -3;
  };

  sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

  bool success = (sqlite3_step(stmt) == SQLITE_DONE);

  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return success ? 0 : -4;
};

bool MA::Database::accessToDatabase(const std::string& hashed_password) {
  sqlite3* db = nullptr;

  bool founded = false;
  std::string password = "";
  
  if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
    if (db) sqlite3_close(db);
    return false;
  };

  std::string sql = "SELECT hashed_password FROM access WHERE hashed_password = ?;";
  sqlite3_stmt* stmt = nullptr;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return false;
  };

  sqlite3_bind_text(stmt, 1, hashed_password.c_str(), -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* pwd = sqlite3_column_text(stmt, 0);

    if (pwd) password = reinterpret_cast<const char*>(pwd);
    founded = true;
    if(password != hashed_password) founded = false;
  };

  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return founded;
};

bool MA::Database::updateDatabaseAccessPassword(const std::string& hashed_password, const std::string& new_hashed_password) {
  sqlite3* db = nullptr;

  if(!accessToDatabase(hashed_password)) return false;

  if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
    if (db) sqlite3_close(db);
    return false;
  };

  std::string sql = "UPDATE access SET hashed_password = ? WHERE hashed_password = ?;";
  sqlite3_stmt* stmt = nullptr;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return false;
  };

  sqlite3_bind_text(stmt, 1, hashed_password.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, new_hashed_password.c_str(), -1, SQLITE_STATIC);

  bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    
  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return success;
};



std::vector<uint8_t> MA::Database::str_to_bytes(const std::string& data) {
  std::vector<uint8_t> bytes;
  for (size_t i = 0; i + 1 < data.length(); i += 2) {
    std::string byteString = data.substr(i, 2);
    bytes.push_back(static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16)));
  };
  return bytes;
};

std::string MA::Database::bytes_to_str(const std::vector<uint8_t>& bytes) {
  std::ostringstream oss;
  for (uint8_t b : bytes) {
    oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
  };
  return oss.str();
};

std::vector<uint8_t> MA::Database::aes_encrypt(const std::vector<uint8_t>& key_bytes, const std::vector<uint8_t>& data_bytes) {
  if (key_bytes.size() != 32) return {};

  unsigned char iv[16];
  if (!RAND_bytes(iv, 16)) return {};

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if (!ctx) return {};

  std::vector<unsigned char> ciphertext(data_bytes.size() + EVP_MAX_BLOCK_LENGTH);
  int len = 0, ciphertext_len = 0;

  if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key_bytes.data(), iv) != 1 || EVP_EncryptUpdate(ctx, ciphertext.data(), &len, data_bytes.data(), static_cast<int>(data_bytes.size())) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return {};
  };

  ciphertext_len = len;
  
  if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return {};
  };

  ciphertext_len += len;
  
  std::vector<uint8_t> combined;
  combined.insert(combined.end(), iv, iv + 16);
  combined.insert(combined.end(), ciphertext.data(), ciphertext.data() + ciphertext_len);

  EVP_CIPHER_CTX_free(ctx);
  return combined;
};

std::vector<uint8_t> MA::Database::aes_decrypt(const std::vector<uint8_t>& key_bytes, const std::vector<uint8_t>& data_bytes) {
  if (key_bytes.size() != 32) return {};
  if (data_bytes.size() < 16) return {};

  const unsigned char* iv = data_bytes.data();
  const unsigned char* ciphertext = data_bytes.data() + 16;
  int ciphertext_len = static_cast<int>(data_bytes.size() - 16);

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if (!ctx) return {};

  std::vector<unsigned char> plaintext(ciphertext_len + EVP_MAX_BLOCK_LENGTH);
  int len = 0, plaintext_len = 0;
  std::vector<uint8_t> decrypted_out;

  if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key_bytes.data(), iv) == 1) {
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext, ciphertext_len) == 1) {
      plaintext_len = len;
      if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) == 1) {
        plaintext_len += len;
        decrypted_out.assign(plaintext.data(), plaintext.data() + plaintext_len);
      };
    };
  };
  
  EVP_CIPHER_CTX_free(ctx);
  return decrypted_out;
};

