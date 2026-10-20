// Password Manager
// Copyright 2026 Daynlight
// Licensed under the GNU General, Version 3.0.
// See LICENSE file for details.



#pragma once
#include <filesystem>



namespace MA::Config {
  inline const std::filesystem::path DB_FILE_PATH = "/data/data/com.password_manager.app/databases/vault.db";
  inline const std::string DEFAULT_DB_PASSWORD = "admin";
};
