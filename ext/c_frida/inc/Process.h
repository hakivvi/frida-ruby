#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cProcess;

void	define_Process();
VALUE	Process_from_FridaProcess(FridaProcess *handle);
