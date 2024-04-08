#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cRelay;

void	define_Relay();
VALUE	Relay_from_FridaRelay(FridaRelay *handle);