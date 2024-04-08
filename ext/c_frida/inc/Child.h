#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cChild;

void	define_Child();
VALUE	Child_from_FridaChild(FridaChild *stream);
