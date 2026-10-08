#pragma once

#include <cstdio>
#include <string>
#include <atomic>
#include <thread>

#include <android/log.h>
#include <android/input.h>
#include <android_native_app_glue.h>

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
    
  std::string m_apiResponse = "Brak danych";
  char m_urlBuffer[256] = "http://jsonplaceholder.typicode.com";
  std::atomic<bool> m_isLoading{false};

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
  void SendApiRequest();

// === Handlers
public:
  static void HandleCmd(struct android_app* app, int32_t cmd);
  static int32_t HandleInput(struct android_app* app, AInputEvent* event);
};
};

