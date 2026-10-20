#include "Core.h"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif



extern IMGUI_IMPL_API int32_t ImGui_ImplAndroid_HandleInputEvent(const AInputEvent* event);



MA::Core::Core(android_app* app) : m_app(app) {
  LOGI("MA::Core Constructor Called");
};

MA::Core::~Core() {
  onEnd();
};


// === Core Events
void MA::Core::onStart() {
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

void MA::Core::onRender() {
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

void MA::Core::onEnd() {
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


// === Getters/Setters
bool MA::Core::IsInitialized() const { 
  return m_initialized; 
};


// === Render
void MA::Core::RenderUI() {
  auto& io = ImGui::GetIO();
    
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(io.DisplaySize);

  ImGuiWindowFlags window_flags = 
    ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNavFocus;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("FullScreenOverlay", nullptr, window_flags);
    
  ImGui::PopStyleVar();
  
  if(scene == 4) RenderLogin();
  else if(scene == 0) RenderScene();
  else if(scene == 1) RenderGetPassword();
  else if(scene == 2) RenderAddPassword();
  else if(scene == 3) RenderRemovePassword();

  ImGui::End();
};

void MA::Core::RenderScene() {
  password = "";
  db_password = "";
  name = "";
  add_state = -1;
  remove_state = -1;
  memset(buffer, 0, sizeof(buffer));

  float windowHeight = ImGui::GetWindowHeight() - padding;
    
  float btnHeight = 50.0f;
  float spacing = 20.0f;
  float totalHeight = (btnHeight * 3) + (spacing * 2);
    
  ImGui::SetCursorPosY((windowHeight - totalHeight) * 0.5f);

  if (CenteredButton("Get Password", 0.8f, btnHeight)) { scene = 1; return; };
  ImGui::Dummy(ImVec2(0, spacing));
  if (CenteredButton("Add Password", 0.8f, btnHeight)) { scene = 2; return; };
  ImGui::Dummy(ImVec2(0, spacing)); 
  if (CenteredButton("Remove Password", 0.8f, btnHeight)) { scene = 3; return; };
};

void MA::Core::RenderLogin() {
  password = "";
  db_password = "";
  name = "";
  add_state = -1;
  remove_state = -1;
  memset(buffer, 0, sizeof(buffer));

  float windowHeight = ImGui::GetWindowHeight() - padding;
    
  float btnHeight = 50.0f;
  float spacing = 20.0f;
  float totalHeight = (btnHeight * 2) + (spacing * 1);
    
  ImGui::SetCursorPosY((windowHeight - totalHeight) * 0.5f);

  CenteredTextInput("Password", password);
  ImGui::Dummy(ImVec2(0, spacing)); 
  if (CenteredButton("Login", 0.8f, btnHeight)) { scene = 0; return; };
};

void MA::Core::RenderGetPassword(){
  float windowHeight = ImGui::GetWindowHeight() - padding;

  float btnHeight = 50.0f;
  float spacing = 20.0f;
  float totalHeight = (btnHeight * 4) + (spacing * 3);
  if(password != "") totalHeight += btnHeight + spacing;
    
  ImGui::SetCursorPosY((windowHeight - totalHeight) * 0.5f);

  if (CenteredButton("Back", 0.8f, btnHeight)) { scene = 0; return; };
  ImGui::Dummy(ImVec2(0, spacing));
  CenteredTextInput("DB Password", db_password);
  ImGui::Dummy(ImVec2(0, spacing));
  CenteredTextInput("Record Name", name);
  ImGui::Dummy(ImVec2(0, spacing));
  if (CenteredButton("Get Password", 0.8f, btnHeight)) password = database.getPassword(db_password, name);
  if(password != ""){
    ImGui::Dummy(ImVec2(0, spacing));
    CenteredText(password);
  };
};

void MA::Core::RenderAddPassword(){
  float windowHeight = ImGui::GetWindowHeight() - padding;
  
  float btnHeight = 50.0f;
  float spacing = 20.0f;
  float totalHeight = (btnHeight * 5) + (spacing * 5);
  if(add_state != -1) totalHeight += btnHeight + spacing;
    
  ImGui::SetCursorPosY((windowHeight - totalHeight) * 0.5f);

  if (CenteredButton("Back", 0.8f, btnHeight)) { scene = 0; return; };
  ImGui::Dummy(ImVec2(0, spacing));
  CenteredTextInput("DB Password", db_password);
  ImGui::Dummy(ImVec2(0, spacing));
  CenteredTextInput("Record Name", name);
  ImGui::Dummy(ImVec2(0, spacing));
  CenteredTextInput("Password", password);
  ImGui::Dummy(ImVec2(0, spacing));
  if (CenteredButton("Add Password", 0.8f, btnHeight)) add_state = database.storePassword(db_password, name, password);

  if(add_state == 0){
    ImGui::Dummy(ImVec2(0, spacing));
    CenteredText("Password "+ name +" Saved");
  }
  else if(add_state == -2){
    ImGui::Dummy(ImVec2(0, spacing));
    CenteredText("Password "+ name +" Encrypted Failed");
  }
  else if(add_state == -3){
    ImGui::Dummy(ImVec2(0, spacing));
    CenteredText("Password "+ name +" Save to database failed"); 
  };
};

void MA::Core::RenderRemovePassword(){
  float windowHeight = ImGui::GetWindowHeight() - padding;
  
  float btnHeight = 50.0f;
  float spacing = 20.0f;
  float totalHeight = (btnHeight * 3) + (spacing * 2);
  if(remove_state != -1) totalHeight += btnHeight + spacing;
    
  ImGui::SetCursorPosY((windowHeight - totalHeight) * 0.5f);

  if (CenteredButton("Back", 0.8f, btnHeight)) { scene = 0; return; };
  ImGui::Dummy(ImVec2(0, spacing));
  CenteredTextInput("Record Name", name);
  ImGui::Dummy(ImVec2(0, spacing));
  if (CenteredButton("Remove Password", 0.8f, btnHeight)) remove_state = database.removePassword(name);
  if(remove_state == 0){
    ImGui::Dummy(ImVec2(0, spacing));
    CenteredText("Password "+ name +" Removed");
  }
  else if(remove_state == -2){
    ImGui::Dummy(ImVec2(0, spacing));
    CenteredText("Password "+ name +" Can't open database");
  }
  else if(remove_state == -3){
    ImGui::Dummy(ImVec2(0, spacing));
    CenteredText("Password "+ name +" Preapare failed");
  }
  else if(remove_state == -4){
    ImGui::Dummy(ImVec2(0, spacing));
    CenteredText("Password "+ name +" end failed");
  };
};


// === Handlers
bool MA::Core::CenteredButton(const char* label, float width_ratio, float height) {
  float windowWidth = ImGui::GetWindowWidth() - padding;
  float btnWidth = windowWidth * width_ratio;
  ImGui::SetCursorPosX((windowWidth - btnWidth) * 0.5f);
  return ImGui::Button(label, ImVec2(btnWidth, height));
};

void MA::Core::CenteredText(const std::string& text) {
  float windowWidth = ImGui::GetWindowWidth() - padding;
  float textWidth = ImGui::CalcTextSize(text.c_str()).x;
  ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
  ImGui::Text("%s", text.c_str());
};

bool MA::Core::CenteredTextInput(const char* label, std::string& data, float width_ratio) {
  float windowWidth = ImGui::GetWindowWidth() - padding;
  float fieldWidth = windowWidth * width_ratio;
  ImGui::SetCursorPosX((windowWidth - fieldWidth) * 0.5f);
  ImGui::SetNextItemWidth(fieldWidth);
  
  static std::string* last_data_ptr = nullptr;
  
  if (last_data_ptr != &data) {
    snprintf(buffer, sizeof(buffer), "%s", data.c_str());
    last_data_ptr = &data;
  };
    
  if (ImGui::InputText(label, buffer, sizeof(buffer))) {
    data = buffer;
    return true;
  };
    
  return false;
};

void MA::Core::HandleCmd(struct android_app* app, int32_t cmd) {
  Core* engine = (Core*)app->userData;
  
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      if (engine->m_app->window != nullptr) engine->onStart();
      break;
    case APP_CMD_TERM_WINDOW:
      engine->onEnd();
      break;
  };
};

int32_t MA::Core::HandleInput(struct android_app* app, AInputEvent* event) {
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


void MA::Core::ToggleAndroidKeyboard(android_app* app, bool show) {
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
