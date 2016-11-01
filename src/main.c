#include "../inc/main.h"

struct timeval timeout;
FILE *topology;
sock temp;
top info;

char buf[MAX_LINE];
char getip[INET_ADDRSTRLEN];
char gethost[MAX_HOST];
char** tokens;

int Num_conn;
unsigned int numTok;
int my_id;
socklen_t sock_len = sizeof(temp.sock_info);

char* getIP() {
	bzero(&temp,sizeof(temp)); 
	if((temp.sock = socket(AF_INET, SOCK_DGRAM,0)) == -1){
		perror("ERROR! CAN NOT GET IP! : ");
		exit(1);
	}
	temp.sock_info.sin_family = AF_INET;
	temp.sock_info.sin_port = htons(53);
	temp.sock_info.sin_addr.s_addr=inet_addr("8.8.8.8");
	if((connect(temp.sock, (struct sockaddr*)&temp.sock_info, sock_len)) == -1) perror("CANNOT GET MY IP:");
	if((getsockname(temp.sock, (struct sockaddr*)&temp.sock_info,&sock_len)) == -1) perror("CanNOT GET SOCK INFO");
	inet_ntop(AF_INET, &temp.sock_info.sin_addr, getip, INET_ADDRSTRLEN);
	return getip;
}
//get public ip using UDP connection USE GOOGLE DNS server 
char* findIP(char* hostname){
	struct hostent *h;
	if((h = gethostbyname(hostname)) ==NULL){
		herror("Cannot resolve IP from hostname..");
		sprintf(getip,"NA");
		return getip;
	}
	memcpy(&temp.sock_info.sin_addr, h->h_addr_list[0], h->h_length);
	inet_ntop(AF_INET, &temp.sock_info.sin_addr, getip, INET_ADDRSTRLEN);
	return getip;
}
// will return hostname from IP address and save to getip
char* findHost(struct sockaddr_in addr){
	struct sockaddr_in t_addr = addr;
	getnameinfo((struct sockaddr*)&t_addr, sock_len, gethost, sizeof(gethost),NULL,0,0);
	return gethost;
}
//get host name based on IP String
int isValidIP(char *ip){
	struct sockaddr_in t;
	int result = inet_pton(AF_INET, ip, &(t.sin_addr));
	return result != 0;
}
//to determine if given string is IPaddress

int top_validate(FILE *top){
	int i;
	getIP();
	//get ip of current machine
	info.num_serv = NUM_SERVER;
	info.num_conn = Num_conn;
	//set initial numbers
	for(i =0; i<NUM_SERVER; i++){
		fgets(buf,sizeof(buf),top);
		tokens = parseTok(buf);
		//read the line and tokenize it
		if(numTok != 3){
			printf("your topology file has problem\n");
			fclose(top);
			exit(1);
		}
		//exit if it has less than 3 tokens
		if((info.serv_info[i].id = atoi(tokens[0]))\
	        && (info.serv_info[i].port=atoi(tokens[2]))){
		//if id and port is not 0, it is not empty 
		   if(isValidIP(tokens[1])) strncpy(info.serv_info[i].ip, tokens[1],INET_ADDRSTRLEN);
		   else{
			if(!findIP(tokens[1])){
			   printf("Host | IP in topology is wrong\n");
			   fclose(top);
			   exit(1);
			}strncpy(info.serv_info[i].ip,getip,INET_ADDRSTRLEN);
		   }
		   //check if given string is valid ipform, ifnot
		   //convert it
		}else{
			printf("value cannot be 0 | char\n");
			fclose(top);
			exit(1);
		}
		if(strcmp(getip,info.serv_info[i].ip)==0){
			my_id = info.serv_info[i].id;
		}
		//record my id
	}
	for(i =0; i<Num_conn; i++){
		fgets(buf,sizeof(buf),top);
		tokens = parseTok(buf);
		if(numTok!=3){
			printf("your topology file has problem\n");
			fclose(top);
			exit(1);
		}
		if(my_id == atoi(tokens[0])){
		    info.conn_info[i].src = atoi(tokens[0]);
		    info.conn_info[i].dst = atoi(tokens[1]);
		}else if(my_id == atoi(tokens[1])) {
		    info.conn_info[i].src = atoi(tokens[1]);
  		    info.conn_info[i].dst =atoi(tokens[0]);	
		}
	}		
}
int main(int argc, char* argv[]){
	if(argc != 5){
		printf(PROC_USAGE);
	//print error if it comes short of number of arguments
	}else{
		if(strcmp("-i",argv[3])==0 && strcmp("-t",argv[1])==0){
		   if(atoi(argv[4])) timeout.tv_sec = atoi(argv[4]);
		   else printf("please give right time interval!\n");
		   //test interval validity
		   topology = fopen(argv[2],"r+");	
		   int i;
	           if(topology){
		      for(i=0; i<2; i++){
			fgets(buf,sizeof(buf),topology);
			if(i) Num_conn = atoi(buf);
		      }
		   }else printf("File could not be open..\n");
		   //test file open validity  
		  top_validate(topology);
		   //test if topology contains correct data
		}else{
			printf(PROC_USAGE);
		}
	}
	return 0;
} 
