/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"

extern VALUE classMapTile;
extern VALUE classMap;

extern void initSpriteClasses();

typedef struct RUDLMapTile{
	double x, y;
	VALUE surface;
} RUDLMapTile;

typedef RUDLMap{
	Uint16 w,h;
	RUDLMapTile** tile;
} RUDLMap;
