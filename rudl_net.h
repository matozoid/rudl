/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */
#include "rudl.h"

extern VALUE classIPAddress;
extern VALUE classTCPSocket;
extern VALUE classUDPSocket;
extern VALUE classUDPPacket;
extern VALUE classSocketSet;

extern void initNetClasses();
extern void startNet();

extern void quitNet();

