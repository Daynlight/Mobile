#include "App.h"
#include <android_native_app_glue.h>

void android_main(struct android_app* state) {
  app_dummy();

  MA::App app;

  int ident;
  int events;
  struct android_poll_source* source;

  while ((ident = ALooper_pollAll(-1, nullptr, &events, (void**)&source)) >= 0) {
    if (source != nullptr) {
      source->process(state, source);
    };
    if (state->destroyRequested != 0) {
      break;
    }
  };
};
