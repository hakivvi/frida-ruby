#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cIOStream;

typedef struct {
    GObject_d		base;
    GInputStream	*input;
    GOutputStream	*output;
} IOStream_d;

typedef struct {
    GOutputStream	*output;
    char			*data;
    gsize			size;
} write_proxy_args;

typedef struct {
    GInputStream	*input;
    char			*buffer;
    gsize			count;
} read_proxy_args;

void IOStream_free(IOStream_d *d);

void	define_IOStream();
VALUE	IOStream_from_GIOStream(GIOStream *stream);
