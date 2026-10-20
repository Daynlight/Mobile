#!/bin/bash

adb exec-out run-as com.password_manager.app cat /data/data/com.password_manager.app/databases/vault.db > vault.db
sqlitebrowser vault.db &

