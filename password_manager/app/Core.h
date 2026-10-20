// Password Manager
// Copyright 2026 Daynlight
// Licensed under the GNU General, Version 3.0.
// See LICENSE file for details.



#pragma once
#include <cstdio>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>

#include <android/log.h>
#include <android/input.h>
#include <android_native_app_glue.h>
#include <android/native_activity.h>
#include <android/window.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#define LOG_TAG "Password_Manager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include "Database.h"



namespace MA {
class Core {
// ==============================================
// === Data =====================================
// ==============================================
private:
  android_app* m_app = nullptr;
    
  EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
  EGLSurface m_eglSurface = EGL_NO_SURFACE;
  EGLContext m_eglContext = EGL_NO_CONTEXT;
  bool m_initialized = false;
  
  int scene = 4;

  MA::Database database;
  std::string db_password = "";
  std::string name = "";
  std::string password = "";
  int add_state = -1;
  int remove_state = -1;
  char buffer[4096] = {};
  float padding = 40.0f;

// ==============================================
// === Functions ================================
// ==============================================
// ==================== //
// === Constructors === //
// ==================== //
public:
  Core(android_app* app);
  ~Core();
  Core(const Core& second) = delete;
  Core& operator=(const Core& second) = delete;
  Core(Core&& second) = delete;
  Core& operator=(Core&& second) = delete;

// ==================== //
// === Core Events ==== //
// ==================== //
public:
  void onStart();
  void onRender();
  void onEnd();

// ==================== //
// === Getters/Setters  //
// ==================== //
  bool IsInitialized() const;

// ==================== //
// === Render ========= //
// ==================== //
private:
  void RenderUI();
  void RenderScene();
  void RenderLogin();
  void RenderAddLogin();
  void RenderRemoveLogin();
  void RenderGetPassword();
  void RenderAddPassword();
  void RenderRemovePassword();

// ==================== //
// === Helpers ======== //
// ==================== //
private:
  bool CenteredButton(const char* label, float width_ratio = 0.8f, float height = 50.0f);
  void CenteredText(const std::string& text);
  bool CenteredTextInput(const char* label, std::string& data, float width_ratio = 0.8f);

// ==================== //
// === Hendlers ======= //
// ==================== //
public:
  static void HandleCmd(struct android_app* app, int32_t cmd);
  static int32_t HandleInput(struct android_app* app, AInputEvent* event);
  void ToggleAndroidKeyboard(android_app* app, bool show);
};
};

