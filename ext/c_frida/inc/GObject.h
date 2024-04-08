#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cGObject;

void	GObject_free(GObject_d *d);

void	define_GObject();
