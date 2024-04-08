#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cScript;
extern VALUE cGObject;

typedef struct {
    FridaScript	*handle;
    uint port;
} enable_debugger_proxy_args;

typedef struct {
    FridaScript	*handle;
    char * message;
    GBytes * data;
} post_proxy_args;

void	define_Script();
VALUE	Script_from_FridaScript(FridaScript *handle);
