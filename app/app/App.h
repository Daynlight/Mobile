#pragma once

#include <cstdio>
#include <android/log.h>

#define LOG_TAG "MA_APP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


namespace MA{
class App{
// =======================
// === Data ==============
// =======================
private:

// =======================
// === Functions =========
// =======================
// === Constructors
public:
  App();
  ~App();
  App(const App& second);
  App& operator=(const App& second);
  App(App&& second);
  App& operator=(App&& second);
};
};
