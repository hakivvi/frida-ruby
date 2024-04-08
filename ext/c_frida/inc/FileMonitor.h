#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cFileMonitor;

void	define_FileMonitor();
VALUE	FileMonitor_from_FridaFileMonitor(FridaFileMonitor *handle);