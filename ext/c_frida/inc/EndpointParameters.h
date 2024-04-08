#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cEndpointParameters;

struct _FridaRBAuthenticationService {
    GObject       parent;
    VALUE         callback;
    GThreadPool   *pool;
};

void	define_EndpointParameters();
VALUE	EndpointParameters_from_FridaEndpointParameters(FridaEndpointParameters *handle);