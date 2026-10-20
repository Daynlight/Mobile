#include "Core.h"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include "httplib.h"



extern IMGUI_IMPL_API int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* event);



MA::App::App(android_app* app) : m_app(app) {
  LOGI("MA::App Constructor Called");
};

MA::App::~App() {
  onEnd();
};


// === App Events
void MA::App::onStart() {
  if (m_initialized) return;
  LOGI("Inicjalizacja EGL i OpenGL ES");

  ANativeActivity_setWindowFlags(m_app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
    
  m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(m_eglDisplay, nullptr, nullptr);

  const EGLint attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 16,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
    EGL_NONE
  };

  EGLConfig config;
  EGLint numConfigs;
  eglChooseConfig(m_eglDisplay, attribs, &config, 1, &numConfigs);

  EGLint format;
  eglGetConfigAttrib(m_eglDisplay, config, EGL_NATIVE_VISUAL_ID, &format);
  ANativeWindow_setBuffersGeometry(m_app->window, 0, 0, format);

  m_eglSurface = eglCreateWindowSurface(m_eglDisplay, config, m_app->window, nullptr);

  const EGLint contextAttribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 3,
    EGL_NONE
  };
  m_eglContext = eglCreateContext(m_eglDisplay, config, nullptr, contextAttribs);

  if (eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext) == EGL_FALSE) {
    LOGE("Błąd podczas eglMakeCurrent!");
    return;
  };

  LOGI("Inicjalizacja ImGui");

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
    
  ImGui::StyleColorsDark();
  ImGui::GetStyle().ScaleAllSizes(3.0f);
  
  io.FontGlobalScale = 3.0f; 
  io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;

  ImGui_ImplAndroid_Init(m_app->window);
  ImGui_ImplOpenGL3_Init("#version 300 es");

  m_initialized = true;
};

void MA::App::onRender() {
  if (!m_initialized) return;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplAndroid_NewFrame();
  ImGui::NewFrame();
  
  ImGuiIO& io = ImGui::GetIO();
  static bool s_keyboardVisible = false;

  if (io.WantTextInput != s_keyboardVisible) {
    s_keyboardVisible = io.WantTextInput;
    ToggleAndroidKeyboard(m_app, s_keyboardVisible);
  }
  RenderUI();

  ImGui::Render();

  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(1.0f, 0.12f, 0.12f, 1.00f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  eglSwapBuffers(m_eglDisplay, m_eglSurface);
};

void MA::App::onEnd() {
  if (!m_initialized) return;

  LOGI("Niszczenie kontekstu EGL i ImGui");

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplAndroid_Shutdown();
  ImGui::DestroyContext();

  if (m_eglDisplay != EGL_NO_DISPLAY) {
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (m_eglContext != EGL_NO_CONTEXT) eglDestroyContext(m_eglDisplay, m_eglContext);
    if (m_eglSurface != EGL_NO_SURFACE) eglDestroySurface(m_eglDisplay, m_eglSurface);
    eglTerminate(m_eglDisplay);
  };

  m_eglDisplay = EGL_NO_DISPLAY;
  m_eglContext = EGL_NO_CONTEXT;
  m_eglSurface = EGL_NO_SURFACE;
  m_initialized = false;
};


// === Render
void MA::App::RenderUI() {
  auto& io = ImGui::GetIO();
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(io.DisplaySize);

  if(register_on) RenderRegister();
  else RenderLogin();
};

void MA::App::RenderRegister() {   
  ImGui::Begin("Mobile App - Panel Rejestracji", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

  ImGui::InputText("Użytkownik", m_username, sizeof(m_username));
  ImGui::InputText("Hasło", m_password, sizeof(m_password), ImGuiInputTextFlags_Password);
  ImGui::InputText("Powtórz hasło", m_validatePassword, sizeof(m_validatePassword), ImGuiInputTextFlags_Password);

  ImGui::Spacing();

  if (ImGui::Button("Zarejestruj", ImVec2(-1, 0)) && !m_isLoading) {
    if (std::string(m_password) != std::string(m_validatePassword)) {
      std::lock_guard<std::mutex> lock(m_apiMutex);
      m_apiResponse = "Hasła nie są identyczne!";
    } else {
      m_isLoading = true;
      std::string jsonBody = "{\"username\":\"" + std::string(m_username) + 
                           "\",\"password\":\"" + std::string(m_password) + 
                           "\",\"validate_password\":\"" + std::string(m_validatePassword) + "\"}";

      std::thread([this, jsonBody]() {
        SendApiRequest("/register", jsonBody);
      }).detach();
    };
  };

  if (ImGui::Button("Masz już konto? Zaloguj się", ImVec2(-1, 0))) register_on = false;

  ImGui::Separator();
    
  if (m_isLoading.load()) {
    ImGui::TextUnformatted("Rejestrowanie...");
  } else {
    std::lock_guard<std::mutex> lock(m_apiMutex);
    if (!m_apiResponse.empty()) {
      ImGui::TextWrapped("Odpowiedź: %s", m_apiResponse.c_str());
    }
  }

  ImGui::End();
};

void MA::App::RenderLogin() {
  ImGui::Begin("Mobile App - Panel Logowania", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

  ImGui::InputText("Użytkownik", m_username, sizeof(m_username));
  ImGui::InputText("Hasło", m_password, sizeof(m_password), ImGuiInputTextFlags_Password);

  ImGui::Spacing();

  if (ImGui::Button("Zaloguj", ImVec2(-1, 0)) && !m_isLoading) {
    m_isLoading = true;
    std::string jsonBody = "{\"username\":\"" + std::string(m_username) + 
                           "\",\"password\":\"" + std::string(m_password) +  "\"}";

    std::thread([this, jsonBody]() {
      SendApiRequest("/login", jsonBody);
    }).detach();
  };

  if (ImGui::Button("Nie masz konta? Zarejestruj się", ImVec2(-1, 0))) register_on = true;

  ImGui::Separator();
    
  if (m_isLoading.load()) {
    ImGui::TextUnformatted("Ładowanie danych...");
  } else {
    std::lock_guard<std::mutex> lock(m_apiMutex);
    if (!m_apiResponse.empty()) {
      ImGui::TextWrapped("Odpowiedź: %s", m_apiResponse.c_str());
    }
  }

  ImGui::End();
};

void MA::App::SendApiRequest(const std::string& endpoint, const std::string& postData) {
  std::string responseText;
    
  httplib::SSLClient cli("10.0.2.2", 3000);
  cli.set_connection_timeout(5, 0); 
  cli.set_read_timeout(5, 0);

  cli.enable_server_certificate_verification(false);

  if (auto res = cli.Post(endpoint.c_str(), postData, "application/json")) {
    if (res->status == 200 || res->status == 201) {
      responseText = res->body;
      LOGI("HTTP Success (%d): %s", res->status, res->body.c_str());
    } else {
      responseText = "Błąd HTTP: " + std::to_string(res->status) + "\n" + res->body;
      LOGE("HTTP Status code: %d", res->status);
    }
  } else {
    auto err = res.error();
    responseText = "Błąd połączenia z serwerem: " + httplib::to_string(err);
    LOGE("HTTP Request failed: %s", httplib::to_string(err).c_str());
  }

  {
    std::lock_guard<std::mutex> lock(m_apiMutex);
    m_apiResponse = std::move(responseText);
  }

  m_isLoading = false;
};


// === Handlers
void MA::App::HandleCmd(struct android_app* app, int32_t cmd) {
  App* engine = (App*)app->userData;
  
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      if (engine->m_app->window != nullptr) engine->onStart();
      break;
    case APP_CMD_TERM_WINDOW:
      engine->onEnd();
      break;
  };
};

int32_t MA::App::HandleInput(struct android_app* app, AInputEvent* event) {
  int32_t eventType = AInputEvent_getType(event);

  if (eventType == AINPUT_EVENT_TYPE_KEY) {
    int32_t action = AKeyEvent_getAction(event);
    int32_t keyCode = AKeyEvent_getKeyCode(event);

    if (action == AKEY_EVENT_ACTION_DOWN) {
      if (keyCode == AKEYCODE_BACK) return 0;

      if (keyCode == AKEYCODE_DEL) {
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Backspace, true);
      }
      else if (keyCode == AKEYCODE_ENTER) {
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, true);
      }
      
      JNIEnv* env = nullptr;
      app->activity->vm->AttachCurrentThread(&env, nullptr);
      
      jclass keyEventClass = env->FindClass("android/view/KeyEvent");
      jmethodID getUnicodeChar = env->GetMethodID(keyEventClass, "getUnicodeChar", "(I)I");
      jmethodID keyEventCons = env->GetMethodID(keyEventClass, "<init>", "(II)V");
      
      jobject keyEventObj = env->NewObject(keyEventClass, keyEventCons, action, keyCode);
      jint unicodeKey = env->CallIntMethod(keyEventObj, getUnicodeChar, AKeyEvent_getMetaState(event));

      if (unicodeKey > 0) {
        ImGui::GetIO().AddInputCharacter((unsigned int)unicodeKey);
      }

      env->DeleteLocalRef(keyEventObj);
      env->DeleteLocalRef(keyEventClass);
      app->activity->vm->DetachCurrentThread();
    } 
    else if (action == AKEY_EVENT_ACTION_UP) {
      if (keyCode == AKEYCODE_DEL) {
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Backspace, false);
      } else if (keyCode == AKEYCODE_ENTER) {
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, false);
      }
    }
  }

  return ImGui_ImplAndroid_HandleInputEvent(event);
};


void MA::App::ToggleAndroidKeyboard(android_app* app, bool show) {
  if (!app || !app->activity) return;

  JNIEnv* env = nullptr;
  app->activity->vm->AttachCurrentThread(&env, nullptr);

  jclass activityClass = env->GetObjectClass(app->activity->clazz);
  jmethodID getSystemService = env->GetMethodID(activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");

  jstring serviceName = env->NewStringUTF("input_method");
  jobject imManager = env->CallObjectMethod(app->activity->clazz, getSystemService, serviceName);

  jclass imClass = env->GetObjectClass(imManager);
  
  if (show) {
    jmethodID toggleSoftInput = env->GetMethodID(imClass, "toggleSoftInput", "(II)V");
    env->CallVoidMethod(imManager, toggleSoftInput, 2 /* SHOW_FORCED */, 0);
  } else {
    jmethodID hideSoftInputFromWindow = env->GetMethodID(imClass, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");
    
    jmethodID getWindow = env->GetMethodID(activityClass, "getWindow", "()Landroid/view/Window;");
    jobject window = env->CallObjectMethod(app->activity->clazz, getWindow);
    jclass windowClass = env->GetObjectClass(window);
    
    jmethodID getDecorView = env->GetMethodID(windowClass, "getDecorView", "()Landroid/view/View;");
    jobject decorView = env->CallObjectMethod(window, getDecorView);
    jclass viewClass = env->GetObjectClass(decorView);
    
    jmethodID getWindowToken = env->GetMethodID(viewClass, "getWindowToken", "()Landroid/os/IBinder;");
    jobject binder = env->CallObjectMethod(decorView, getWindowToken);

    env->CallBooleanMethod(imManager, hideSoftInputFromWindow, binder, 0);
    
    env->DeleteLocalRef(binder);
    env->DeleteLocalRef(viewClass);
    env->DeleteLocalRef(decorView);
    env->DeleteLocalRef(windowClass);
    env->DeleteLocalRef(window);
  }

  env->DeleteLocalRef(imClass);
  env->DeleteLocalRef(imManager);
  env->DeleteLocalRef(serviceName);
  env->DeleteLocalRef(activityClass);

  app->activity->vm->DetachCurrentThread();
}
