//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Networking module which uses SDL_net
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"
#include "net_defs.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_sdl.h"
#include "z_zone.h"
#include "main.h"
#include "net_client.h"
#include "net_server.h"
#include "pros/apix.h"

//
// NETWORKING
//

//#include <SDL_net.h>

#define DEFAULT_PORT 2342
static boolean initted = false;
static boolean server = false;

static queue_t loopback_buffer;
/*

static int port = DEFAULT_PORT;
static UDPsocket udpsocket;
static UDPpacket *recvpacket;

typedef struct
{
    net_addr_t net_addr;
    IPaddress sdl_addr;
} addrpair_t;

static addrpair_t **addr_table;
static int addr_table_size = -1;

// Initializes the address table

static void NET_SDL_InitAddrTable(void)
{
    addr_table_size = 16;

    addr_table = Z_Malloc(sizeof(addrpair_t *) * addr_table_size,
                          PU_STATIC, 0);
    memset(addr_table, 0, sizeof(addrpair_t *) * addr_table_size);
}

static boolean AddressesEqual(IPaddress *a, IPaddress *b)
{
    return a->host == b->host
        && a->port == b->port;
}

// Finds an address by searching the table.  If the address is not found,
// it is added to the table.

static net_addr_t *NET_SDL_FindAddress(IPaddress *addr)
{
    addrpair_t *new_entry;
    int empty_entry = -1;
    int i;

    if (addr_table_size < 0)
    {
        NET_SDL_InitAddrTable();
    }

    for (i=0; i<addr_table_size; ++i)
    {
        if (addr_table[i] != NULL
         && AddressesEqual(addr, &addr_table[i]->sdl_addr))
        {
            return &addr_table[i]->net_addr;
        }

        if (empty_entry < 0 && addr_table[i] == NULL)
            empty_entry = i;
    }

    // Was not found in list.  We need to add it.

    // Is there any space in the table? If not, increase the table size

    if (empty_entry < 0)
    {
        addrpair_t **new_addr_table;
        int new_addr_table_size;

        // after reallocing, we will add this in as the first entry
        // in the new block of memory

        empty_entry = addr_table_size;
        
        // allocate a new array twice the size, init to 0 and copy 
        // the existing table in.  replace the old table.

        new_addr_table_size = addr_table_size * 2;
        new_addr_table = Z_Malloc(sizeof(addrpair_t *) * new_addr_table_size,
                                  PU_STATIC, 0);
        memset(new_addr_table, 0, sizeof(addrpair_t *) * new_addr_table_size);
        memcpy(new_addr_table, addr_table, 
               sizeof(addrpair_t *) * addr_table_size);
        Z_Free(addr_table);
        addr_table = new_addr_table;
        addr_table_size = new_addr_table_size;
    }

    // Add a new entry
    
    new_entry = Z_Malloc(sizeof(addrpair_t), PU_STATIC, 0);

    new_entry->sdl_addr = *addr;
    new_entry->net_addr.refcount = 0;
    new_entry->net_addr.handle = &new_entry->sdl_addr;
    new_entry->net_addr.module = &net_sdl_module;

    addr_table[empty_entry] = new_entry;

    return &new_entry->net_addr;
}

static void NET_SDL_FreeAddress(net_addr_t *addr)
{
    int i;
    
    for (i=0; i<addr_table_size; ++i)
    {
        if (addr == &addr_table[i]->net_addr)
        {
            Z_Free(addr_table[i]);
            addr_table[i] = NULL;
            return;
        }
    }

    I_Error("NET_SDL_FreeAddress: Attempted to remove an unused address!");
}

static boolean NET_SDL_InitClient(void)
{
    int p;

    if (initted)
        return true;

    //!
    // @category net
    // @arg <n>
    //
    // Use the specified UDP port for communications, instead of 
    // the default (2342).
    //

    p = M_CheckParmWithArgs("-port", 1);
    if (p > 0)
        port = atoi(myargv[p+1]);

    SDLNet_Init();

    udpsocket = SDLNet_UDP_Open(0);

    if (udpsocket == NULL)
    {
        I_Error("NET_SDL_InitClient: Unable to open a socket!");
    }
    
    recvpacket = SDLNet_AllocPacket(1500);

#ifdef DROP_PACKETS
    srand(time(NULL));
#endif

    initted = true;

    return true;
}

static boolean NET_SDL_InitServer(void)
{
    int p;

    if (initted)
        return true;

    p = M_CheckParmWithArgs("-port", 1);
    if (p > 0)
        port = atoi(myargv[p+1]);

    SDLNet_Init();

    udpsocket = SDLNet_UDP_Open(port);

    if (udpsocket == NULL)
    {
        I_Error("NET_SDL_InitServer: Unable to bind to port %i", port);
    }

    recvpacket = SDLNet_AllocPacket(1500);
#ifdef DROP_PACKETS
    srand(time(NULL));
#endif

    initted = true;

    return true;
}

static void NET_SDL_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    UDPpacket sdl_packet;
    IPaddress ip;
   
    if (addr == &net_broadcast_addr)
    {
        SDLNet_ResolveHost(&ip, NULL, port);
        ip.host = INADDR_BROADCAST;
    }
    else
    {
        ip = *((IPaddress *) addr->handle);
    }

#if 0
    {
        static int this_second_sent = 0;
        static int lasttime;

        this_second_sent += packet->len + 64;

        if (I_GetTime() - lasttime > TICRATE)
        {
            printf("%i bytes sent in the last second\n", this_second_sent);
            lasttime = I_GetTime();
            this_second_sent = 0;
        }
    }
#endif

#ifdef DROP_PACKETS
    if ((rand() % 4) == 0)
        return;
#endif

    sdl_packet.channel = 0;
    sdl_packet.data = packet->data;
    sdl_packet.len = packet->len;
    sdl_packet.address = ip;

    if (!SDLNet_UDP_Send(udpsocket, -1, &sdl_packet))
    {
        I_Error("NET_SDL_SendPacket: Error transmitting packet: %s",
                SDLNet_GetError());
    }
}

static boolean NET_SDL_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    int result;

    result = SDLNet_UDP_Recv(udpsocket, recvpacket);

    if (result < 0)
    {
        I_Error("NET_SDL_RecvPacket: Error receiving packet: %s",
                SDLNet_GetError());
    }

    // no packets received

    if (result == 0)
        return false;

    // Put the data into a new packet structure

    *packet = NET_NewPacket(recvpacket->len);
    memcpy((*packet)->data, recvpacket->data, recvpacket->len);
    (*packet)->len = recvpacket->len;

    // Address

    *addr = NET_SDL_FindAddress(&recvpacket->address);

    return true;
}

void NET_SDL_AddrToString(net_addr_t *addr, char *buffer, int buffer_len)
{
    IPaddress *ip;
    uint32_t host;
    uint16_t port;

    ip = (IPaddress *) addr->handle;
    host = SDLNet_Read32(&ip->host);
    port = SDLNet_Read16(&ip->port);

    M_snprintf(buffer, buffer_len, "%i.%i.%i.%i",
               (host >> 24) & 0xff, (host >> 16) & 0xff,
               (host >> 8) & 0xff, host & 0xff);

    // If we are using the default port we just need to show the IP address,
    // but otherwise we need to include the port. This is important because
    // we use the string representation in the setup tool to provided an
    // address to connect to.
    if (port != DEFAULT_PORT)
    {
        char portbuf[10];
        M_snprintf(portbuf, sizeof(portbuf), ":%i", port);
        M_StringConcat(buffer, portbuf, buffer_len);
    }
}

net_addr_t *NET_SDL_ResolveAddress(const char *address)
{
    IPaddress ip;
    char *addr_hostname;
    int addr_port;
    int result;
    char *colon;

    colon = strchr(address, ':');

    addr_hostname = M_StringDuplicate(address);
    if (colon != NULL)
    {
	addr_hostname[colon - address] = '\0';
	addr_port = atoi(colon + 1);
    }
    else
    {
	addr_port = port;
    }
    
    result = SDLNet_ResolveHost(&ip, addr_hostname, addr_port);

    free(addr_hostname);

    if (result)
    {
        // unable to resolve

        return NULL;
    }
    else
    {
        return NET_SDL_FindAddress(&ip);
    }
}
*/
// Complete module

#define PROTOCOL_SIZE 4
#define START_BYTE 0x33

extern uint8_t DOOM_VEXlink_port;

net_addr_t vexlink_addr = {
    &net_sdl_module, 0, 0
};

static boolean NET_SDL_InitClient(void)
{
    if (initted)
        return true;

    // port, link_id, link_type
    link_init_override(DOOM_VEXlink_port, "DOOM", E_LINK_RECIEVER);
    fprintf(stderr, "awaiting connection to server\n");
    while (!link_connected(DOOM_VEXlink_port)) {
      delay(20);
    }
    fprintf(stderr, "connected\n");

    initted = true;

    return true;
}

static boolean NET_SDL_InitServer(void)
{
    if (initted)
        return true;

    server = true;
    //loopback_buffer = queue_create (50, sizeof(net_packet_t) );

    // port, link_id, link_type
    link_init_override(DOOM_VEXlink_port, "DOOM", E_LINK_TRANSMITTER);
    fprintf(stderr, "awaiting connection from client\n");
    while (!link_connected(DOOM_VEXlink_port)) {
      delay(20);
    }
    fprintf(stderr, "connected\n");

    initted = true;

    return true;
}

static void NET_SDL_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    /*
    if (server){
        queue_append ( loopback_buffer, packet, 0 );
        
        return;
    }
    */
    
    //int res = link_transmit(DOOM_VEXlink_port, (void*)packet->data, (uint16_t) packet->len);
    uint8_t checksum = START_BYTE;
    uint8_t size_tx_buf[2];
    size_tx_buf[1] = (packet->len >> 8) & 0xff;
    size_tx_buf[0] = (packet->len) & 0xff;
    checksum ^= size_tx_buf[1];
    checksum ^= size_tx_buf[0];
    
    for(int i = 0; i < packet->len; i++) {
        checksum ^= ((uint8_t*)packet->data)[i];
    }
    uint32_t rtv = 0;
    // send protocol
    int start_byte = START_BYTE;
    link_transmit_raw(DOOM_VEXlink_port, (void*)&start_byte, 1);
    link_transmit_raw(DOOM_VEXlink_port, size_tx_buf, 2);
    link_transmit_raw(DOOM_VEXlink_port, (void*)packet->data, packet->len);
    link_transmit_raw(DOOM_VEXlink_port, &checksum, 1);

    /*
    printf("\nsending %i: ", packet->len);
    for (int i = 0; i < packet->len; i++)
            printf("%X ", packet->data[i]);
    printf("\n");
    */
    /*
    if ( res == PROS_ERR)
    {
        return;
        //I_Error("NET_SDL_SendPacket: Error transmitting packet: %d",
        //        res);
    }
    */
    
}

#define BUFFER_SIZE 300
char data_buffer[BUFFER_SIZE];

int buffer_tail = 0;

static boolean NET_SDL_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    uint32_t raw_size = link_raw_receivable_size(DOOM_VEXlink_port);
    if (raw_size == 0)
        return false;

    link_receive_raw(DOOM_VEXlink_port, (void*) (&data_buffer)+buffer_tail, raw_size);
    buffer_tail+= raw_size;

    /*
    printf("\n");
    for(int i = 0; i < buffer_tail; i++) {
        printf("%X ", data_buffer[i]);
    }
    printf("recieved %i raw bytes\n", raw_size);
    */

    if (buffer_tail < 3)
        return false;

    int buffer_head = 0;
    for (buffer_head = 0; buffer_head<BUFFER_SIZE;buffer_head++){
        if (data_buffer[buffer_head] == START_BYTE)
            break;
    }
    if (buffer_tail - buffer_head < 2)
        return false;

    uint16_t received_data_size = data_buffer[buffer_head+1] + (data_buffer[buffer_head+2] << 8);

    if (buffer_tail - buffer_head < received_data_size+3)
        return false;

    char data[300];
    memcpy(data, data_buffer + buffer_head+3, received_data_size);

    uint8_t received_checksum = data_buffer[buffer_head+received_data_size+3];

    uint8_t calculated_checksum = START_BYTE;
    calculated_checksum ^= (received_data_size >> 8) & 0xff;
    calculated_checksum ^= (received_data_size) & 0xff;
    for(int i = 0; i < received_data_size; i++) {
        calculated_checksum ^= ((uint8_t*)data)[i];
    }

    if(calculated_checksum != received_checksum ) {
        return false;
    }

    /*
    printf("\nrecvd %i: \n", received_data_size);

    for(int i = 0; i < received_data_size; i++) {
        printf("%X ", data[i]);
    }
    */

    // Put the data into a new packet structure
    *packet = NET_NewPacket(received_data_size);
    memcpy((*packet)->data, data, received_data_size);
    (*packet)->len = received_data_size;
    //}

    // move all the stuff after this packet to the start
    memcpy(data_buffer, data_buffer+buffer_head+received_data_size+4, BUFFER_SIZE-(buffer_head+received_data_size+4));
    *addr = &vexlink_addr;
    buffer_tail -= buffer_head+received_data_size+4;
    return true;


}

void NET_SDL_AddrToString(net_addr_t *addr, char *buffer, int buffer_len){


    M_snprintf(buffer, buffer_len, "1.2.3.4");
};

static void NET_SDL_FreeAddress(net_addr_t *addr){};


net_addr_t *NET_SDL_ResolveAddress(const char *address){
    return &vexlink_addr;
};

net_module_t net_sdl_module =
{
    
    NET_SDL_InitClient,
    NET_SDL_InitServer,
    NET_SDL_SendPacket,
    NET_SDL_RecvPacket,
    NET_SDL_AddrToString,
    NET_SDL_FreeAddress,
    NET_SDL_ResolveAddress,
    
};

