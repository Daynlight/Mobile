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

#define LOG_TAG "MA_APP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)



namespace MA {
class App {
// ==========================
// === Data =================
// ==========================
private:
  android_app* m_app = nullptr;
    
  EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
  EGLSurface m_eglSurface = EGL_NO_SURFACE;
  EGLContext m_eglContext = EGL_NO_CONTEXT;
  bool m_initialized = false;
    
  bool register_on = false;
  std::atomic<bool> m_isLoading{false};
  std::mutex m_apiMutex;
  std::string m_apiResponse;
  std::string token = "";
  char m_username[64] = "";
  char m_password[64] = "";
  char m_validatePassword[64] = "";

// ==========================
// === Functions ============
// ==========================
// === Constructors
public:
  App(android_app* app);
  ~App();
  App(const App& second) = delete;
  App& operator=(const App& second) = delete;
  App(App&& second) = delete;
  App& operator=(App&& second) = delete;

// === App Events
public:
  void onStart();
  void onRender();
  void onEnd();
  bool IsInitialized() const { return m_initialized; }

// === Render
private:
  void RenderUI();
  void RenderRegister();
  void RenderLogin();
  void SendApiRequest(const std::string& endpoint, const std::string& post);

// === Handlers
public:
  static void HandleCmd(struct android_app* app, int32_t cmd);
  static int32_t HandleInput(struct android_app* app, AInputEvent* event);
  void ToggleAndroidKeyboard(android_app* app, bool show);
};
};

