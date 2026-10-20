#!/bin/bash

mkdir -p backend/cert

openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
  -keyout backend/cert/server.key \
  -out backend/cert/server.crt \
  -subj "/CN=localhost"

sudo chmod -R 644 backend/cert/
