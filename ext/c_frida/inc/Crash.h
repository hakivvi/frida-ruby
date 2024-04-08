#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cCrash;

void	define_Crash();
VALUE	Crash_from_FridaCrash(FridaCrash *handle);