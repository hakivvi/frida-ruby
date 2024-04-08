#pragma once

#include "c_frida.h"

extern VALUE mCFrida;
extern VALUE cPortalMembership;

void	define_PortalMembership();
VALUE	PortalMembership_from_FridaPortalMembership(FridaPortalMembership *handle);