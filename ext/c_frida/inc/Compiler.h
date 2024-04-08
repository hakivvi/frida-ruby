#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cCompiler;

typedef struct {
    FridaCompiler   *handle;
    char            *entrypoint;
    FridaBuildOptions *options;
} build_sync_proxy_args;

typedef struct {
    FridaCompiler   *handle;
    char            *entrypoint;
    FridaWatchOptions *options;
} watch_sync_proxy_args;

void	define_Compiler();
VALUE	Compiler_from_FridaCompiler(FridaCompiler *handle);