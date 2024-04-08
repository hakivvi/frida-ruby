#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cApplication;

void	define_Application();
VALUE	Application_from_FridaApplication(FridaApplication *handle);
