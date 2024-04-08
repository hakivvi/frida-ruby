#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cBus;

typedef struct {
    FridaBus	*handle;
    char		*message;
    GBytes		*data;
} bus_post_proxy_args;

void	define_Bus();
VALUE	Bus_from_FridaBus(FridaBus *fridabus);
