/* RUDL - a C library wrapping SDL for use in Ruby. Copyright (C) 2001  Danny van Bruggen */

// DONT_GENERATE_RD2_DOCS

#include "rudl_net.h"
/*

VALUE classNet;
VALUE classTCPSocket;
VALUE classUDPSocket;
VALUE classUDPPacket;
VALUE classSocketSet;

#ifdef HAVE_SDL_NET_H

// WTF is SDLCALL used for??
#define SDLCALL

#include "SDL_net.h"

#define SDLNET_RAISE {rb_raise(classSDLError, SDLNet_GetError());}

void startNet()
{
	if(SDLNet_Init()<0){
		SDLNET_RAISE;
	}
}

void quitNet()
{
	SDLNet_Quit();
}

#endif


static VALUE ipaddress_get_hostname(VALUE self)
{
}

static VALUE ipaddress_hostname_to_ip(VALUE self, VALUE address)
{
	return rb_not_implemented();
}

// TCPSOCKET
#define GETTCPSOCKET _TCPsocket socket; Data_Get_Struct(self, _TCPsocket, socket);

static VALUE createTCPSocketObject(VALUE self, VALUE address, VALUE port)
{
	VALUE newObject;
	IPaddress address;
	

	_TCPsocket* socket=SDLNet_TCP_Open(address.to_i, port);
	SDL_ASSERT(socket);
	newObject=Data_Wrap_Struct(classTCPSocket, 0, SDLNet_TCP_Close, socket);
	return newObject;
}

classTCPSocket=rb_define_class_under(moduleRUDL, "TCPSocket", rb_cObject);
rb_define_singleton_method(classTCPSocket, "new", tcp_socket_new, 2);
rb_define_method(classTCPSocket, "destroy", tcp_socket_destroy, 0);
rb_define_method(classTCPSocket, "accept", tcp_socket_accept, 0);
rb_define_method(classTCPSocket, "peer_address", tcp_socket_peer_address, 0);
rb_define_method(classTCPSocket, "send", tcp_socket_send, 1);
rb_define_method(classTCPSocket, "receive", tcp_socket_receive, 1);
rb_define_method(classTCPSocket, "ready?", tcp_socket_get_ready, 0);

// UDPSOCKET
#define GETUDPSOCKET UDPsocket* socket; Data_Get_Struct(self, UDPsocket, socket);
classUDPSocket=rb_define_class_under(moduleRUDL, "UDPSocket", rb_cObject);
rb_define_singleton_method(classUDPSocket, "new", udp_socket_new, 1);
rb_define_method(classUDPSocket, "destroy", udp_socket_destroy, 0);
rb_define_method(classUDPSocket, "bind", udp_socket_bind, 3);
rb_define_method(classUDPSocket, "unbind", udp_socket_unbind, 1);
rb_define_method(classUDPSocket, "peer_address", udp_socket_peer_address, 0);
rb_define_method(classUDPSocket, "send", udp_socket_send, 1);
rb_define_method(classUDPSocket, "receive", udp_socket_receive, 0);
rb_define_method(classUDPSocket, "ready?", udp_socket_get_ready, 0);

// UDPPACKET
#define GETUDPPACKET UDPpacket* socket; Data_Get_Struct(self, UDPpacket, socket);
classUDPPacket=rb_define_class_under(moduleRUDL, "UDPPacket", rb_cObject);
rb_define_singleton_method(classUDPPacket, "new", udp_packet_new, 1);
rb_define_const(classUDPPacket, "MaxChannels", UINT2NUM(SDLNET_MAX_UDPCHANNELS));
rb_define_const(classUDPPacket, "MaxAddressesPerChannel", UINT2NUM(SDLNET_MAX_UDPADDRESSES));
rb_define_method(classUDPPacket, "channel", udp_packet_channel, 0);
rb_define_method(classUDPPacket, "channel=", udp_packet_set_channel, 1);
rb_define_method(classUDPPacket, "size", udp_packet_get_size, 0);
rb_define_method(classUDPPacket, "size=", udp_packet_set_size, 1);
rb_define_method(classUDPPacket, "status", udp_packet_get_status, 0);
rb_define_method(classUDPPacket, "ip", udp_packet_get_ip, 0);
rb_define_method(classUDPPacket, "ip=", udp_packet_set_ip, 1);
rb_define_method(classUDPPacket, "port", udp_packet_get_port, 0);
rb_define_method(classUDPPacket, "port=", udp_packet_set_port, 1);
rb_define_method(classUDPPacket, "data", udp_packet_get_data, 0);
rb_define_method(classUDPPacket, "data=", udp_packet_set_data, 1);
rb_define_method(classUDPPacket, "resize_and_set", udp_packet_resize_and_set, 1);

// SOCKETSET
#define GETSOCKETSET SDLNet_SocketSet* set; Data_Get_Struct(self, SDLNet_SocketSet, set);
classSocketSet=rb_define_class_under(moduleRUDL, "SocketSet", rb_cObject);
rb_define_singleton_method(classSocketSet, "new", socket_set_new, 1);
rb_define_method(classSocketSet, "push", socket_set_push, 1);
rb_define_method(classSocketSet, "pop", socket_set_pop, 1);
rb_define_method(classSocketSet, "data_available?", socket_set_get_data_available, 0);

/////////////// INIT
*/
void initNetClasses()
{
/*
#ifdef HAVE_SDL_NET_H

	// IPADDRESS
	classIPAddress=rb_define_class_under(moduleRUDL, "IPAddress", rb_cObject);
	rb_eval_string(
			"module RUDL class IPAddress						\n"
			"def initialize(address=nil);set(address);end		\n"
			"def set(address)									\n"
			"	case address									\n"
			"		when NilClass								\n"
			"			@address=0								\n"
			"		when String													\n"
			"			address=IPAddress::hostname_to_ip(address)				\n"
			"			address=address.split('.').collect! {|i| i.to_i}		\n"
			"			@address=address.pack('CCCC')							\n"
			"		when Array									\n"
			"			@address=address.pack('CCCC')			\n"
			"		when IPAddress								\n"
			"			@address=address.to_i					\n"
			"	end												\n"
			"	@address_number=@address.unpack('I')[0]			\n"
			"end												\n"
			"def to_i;@address_number;end						\n"
			"def to_s;to_a.join('.');end										\n"
			"def to_a;array=[];@address.each_byte {|b| array.push b};array;end	\n"
	);
	rb_define_method(classIPAddress, "hostname?", ipaddress_get_hostname, 0);
	rb_define_singleton_method(classIPAddress, "hostname_to_ip", ipaddress_hostname_to_ip, 1);

	// TCPSOCKET
	classTCPSocket=rb_define_class_under(moduleRUDL, "TCPSocket", rb_cObject);
	rb_define_singleton_method(classTCPSocket, "new", tcp_socket_new, 2);
	rb_define_method(classTCPSocket, "destroy", tcp_socket_destroy, 0);
	rb_define_method(classTCPSocket, "accept", tcp_socket_accept, 0);
	rb_define_method(classTCPSocket, "peer_address", tcp_socket_peer_address, 0);
	rb_define_method(classTCPSocket, "send", tcp_socket_send, 1);
	rb_define_method(classTCPSocket, "receive", tcp_socket_receive, 1);
	rb_define_method(classTCPSocket, "ready?", tcp_socket_get_ready, 0);

	// UDPSOCKET
	classUDPSocket=rb_define_class_under(moduleRUDL, "UDPSocket", rb_cObject);
	rb_define_singleton_method(classUDPSocket, "new", udp_socket_new, 1);
	rb_define_method(classUDPSocket, "destroy", udp_socket_destroy, 0);
	rb_define_method(classUDPSocket, "bind", udp_socket_bind, 3);
	rb_define_method(classUDPSocket, "unbind", udp_socket_unbind, 1);
	rb_define_method(classUDPSocket, "peer_address", udp_socket_peer_address, 0);
	rb_define_method(classUDPSocket, "send", udp_socket_send, 1);
	rb_define_method(classUDPSocket, "receive", udp_socket_receive, 0);
	rb_define_method(classUDPSocket, "ready?", udp_socket_get_ready, 0);
	
	// UDPPACKET
	classUDPPacket=rb_define_class_under(moduleRUDL, "UDPPacket", rb_cObject);
	rb_define_singleton_method(classUDPPacket, "new", udp_packet_new, 1);
	rb_define_const(classUDPPacket, "MaxChannels", UINT2NUM(SDLNET_MAX_UDPCHANNELS));
	rb_define_const(classUDPPacket, "MaxAddressesPerChannel", UINT2NUM(SDLNET_MAX_UDPADDRESSES));
	rb_define_method(classUDPPacket, "channel", udp_packet_channel, 0);
	rb_define_method(classUDPPacket, "channel=", udp_packet_set_channel, 1);
	rb_define_method(classUDPPacket, "size", udp_packet_get_size, 0);
	rb_define_method(classUDPPacket, "size=", udp_packet_set_size, 1);
	rb_define_method(classUDPPacket, "status", udp_packet_get_status, 0);
	rb_define_method(classUDPPacket, "ip", udp_packet_get_ip, 0);
	rb_define_method(classUDPPacket, "ip=", udp_packet_set_ip, 1);
	rb_define_method(classUDPPacket, "port", udp_packet_get_port, 0);
	rb_define_method(classUDPPacket, "port=", udp_packet_set_port, 1);
	rb_define_method(classUDPPacket, "data", udp_packet_get_data, 0);
	rb_define_method(classUDPPacket, "data=", udp_packet_set_data, 1);
	rb_define_method(classUDPPacket, "resize_and_set", udp_packet_resize_and_set, 1);
	
	// SOCKETSET
	classSocketSet=rb_define_class_under(moduleRUDL, "SocketSet", rb_cObject);
	rb_define_singleton_method(classSocketSet, "new", socket_set_new, 1);
	rb_define_method(classSocketSet, "push", socket_set_push, 1);
	rb_define_method(classSocketSet, "pop", socket_set_pop, 1);
	rb_define_method(classSocketSet, "data_available?", socket_set_get_data_available, 0);

#endif
*/
}
