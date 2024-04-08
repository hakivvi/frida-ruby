#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cSpawn;

void	define_Spawn();
VALUE	Spawn_from_FridaSpawn(FridaSpawn *stream);
