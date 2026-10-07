#include "App.h"

MA::App::App(){
  LOGI("MA::App Constructor Called");
};

MA::App::~App(){
};

MA::App::App(const App& second){

};

MA::App& MA::App::operator=(const App& second){
  return *this;
};

MA::App::App(App&& second){
};

MA::App& MA::App::operator=(App&& second){
  return *this;
};
