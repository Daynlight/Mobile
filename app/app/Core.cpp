#include "Core.h"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
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

  RenderUI();

  ImGui::Render();

  auto& io = ImGui::GetIO();
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
    
  ImGui::Begin("Mobile App - Control Panel", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

  ImGui::Text("Test komunikacji HTTP:");
  ImGui::InputText("Host", m_urlBuffer, sizeof(m_urlBuffer));

  if (ImGui::Button("Wyślij zapytanie API", ImVec2(-1, 0)) && !m_isLoading) {
    m_isLoading = true;
    m_apiResponse = "Pobieranie danych...";

    std::thread([this]() {
      SendApiRequest();
    }).detach();
  };

  ImGui::Separator();
    
  if (m_isLoading) {
    ImGui::TextUnformatted("Ładowanie...");
  } else {
    ImGui::TextWrapped("Odpowiedź: %s", m_apiResponse.c_str());
  };

  ImGui::End();
};

void MA::App::SendApiRequest() {
  httplib::Client cli(m_urlBuffer);
  cli.set_connection_timeout(5, 0); 

  if (auto res = cli.Get("/todos/1")) {
    if (res->status == 200) {
      m_apiResponse = res->body;
      LOGI("HTTP Success: %s", res->body.c_str());
    } else {
      m_apiResponse = "Błąd HTTP: " + std::to_string(res->status);
      LOGE("HTTP Status code: %d", res->status);
    }
  } else {
    m_apiResponse = "Błąd połączenia z serwerem!";
    LOGE("HTTP Request failed");
  };

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

int32_t App::HandleInput(struct android_app* app, AInputEvent* event) {
  return ImGui_ImplAndroid_HandleInputEvent(event);
};

