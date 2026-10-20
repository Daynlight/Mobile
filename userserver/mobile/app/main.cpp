#include "Core.h"
#include <android_native_app_glue.h>



void android_main(struct android_app* state) {
  MA::App app(state);
  state->userData = &app;
  state->onAppCmd = MA::App::HandleCmd;
  state->onInputEvent = MA::App::HandleInput;

  int ident;
  int events;
  struct android_poll_source* source;

  while (true) {
    while ((ident = ALooper_pollAll(0, nullptr, &events, (void**)&source)) >= 0) {
      if (source != nullptr) source->process(state, source);
      if (state->destroyRequested != 0) {
        app.onEnd();
        return;
      };
    };
    
    if (app.IsInitialized()) app.onRender();
  };
};

