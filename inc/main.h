#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PROC_USAGE "\nYour input arguments are wrong...\nPlease follow the format\n\n ./PA1 -t [filename] -i [Interval]\n\n"
#define USAGE "\n[USAGE]\nupdate <server-ID1> <server-ID2> <Link Cost>\n   - this will update cost of two machines in  local topology\nstep\n   - send routing update to neighbors now!\npackets\n   - display the number of distance vector packets this server has received since the last invocation of this information\ndisplay\n   - display the current routing table! dst ids are sorted\ndisable <server-ID>\n   - disable the link to a given server ID, basically will set cost to infinite cost\ncrash\n   - Close all connection...\n"
#define MAX_LINE 128
#define MAX_INPUT 60
#define MAX_HOST 64
#define MAX_TOK 5
#define MAX_WAIT 3
#define NUM_SERVER 5
#define inf 0xFFFF
#define ip_inf 0xFFFFFFFF
#define zero 0x0000
extern unsigned int numTok;
extern int Num_conn;
//____________________________________________//
//______________Routing table structure______//
typedef struct{
	int id;
	char ip[INET_ADDRSTRLEN];
	unsigned short int port;
}server;

typedef struct{
	int src;
	int dst;
	uint16_t metric;
}connection;

typedef struct{
	int num_serv;
	int num_conn;
	server serv[NUM_SERVER];
	connection conn[NUM_SERVER];
	int neig[NUM_SERVER];
}top;
//______________________________________________//

//_______________________________________________//
//_______________update packet structure________//
typedef struct{
	 uint16_t num_update;
	 uint16_t srv_port;
	 uint32_t srv_ip;	
	 uint32_t srvip_n[NUM_SERVER];
	 uint16_t srvportn[NUM_SERVER];
	 uint16_t pad[NUM_SERVER];
	 uint16_t idn[NUM_SERVER];	
	 uint16_t metric[NUM_SERVER];
}packet;
//______________________________________________//

typedef struct{
	int sock;
	struct sockaddr_in sock_info;
}sock;
char* readLine(void);
char** parseTok(char* input);

#endif
