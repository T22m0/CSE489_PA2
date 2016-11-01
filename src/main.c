#include "../inc/main.h"
//______________________________//
struct timeval timeout;
FILE *topology;
packet pack;
sock temp;
top info;
//__________Main________________//
//________________________________//
char buf[MAX_LINE];
char getip[INET_ADDRSTRLEN];
char gethost[MAX_HOST];
char** tokens;
//__________BUFFER______________//
//_______________________________//
int Num_conn;
unsigned int numTok;
int my_id;
socklen_t sock_len = sizeof(temp.sock_info);
//__________VARIABLES______________//
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
void sortConnInfo(connection c[]){
	int i,j;
	connection temp;
	for(i=0; i<info.num_serv; i++){	     
	   for(j=i+1; j<info.num_serv; j++){
      	     if(info.conn_info[i].dst > info.conn_info[j].dst){
		temp = info.conn_info[i];
		info.conn_info[i]=info.conn_info[j];
		info.conn_info[j]=temp;	
	     }
	   }  	
	}
}
//sorting conn_info in order of dst O(N^2) doesn matter in this case because number is small
void syncTable(FILE* top){
	int i;
	rewind(top);
	//heading beginning of the fil
	fprintf(top, "%d\n%d\n",info.num_serv, info.num_conn);
	//num_conn -1 because it was added for self connection
	for(i =0; i<info.num_serv;i++){
		fprintf(top, "%d %s %d\n",info.serv_info[i].id,\
					  info.serv_info[i].ip,\
					  info.serv_info[i].port);
	}
	for(i =1; i<info.num_serv; i++){
	   if(info.conn_info[i].metric != inf){
		fprintf(top, "%d %d %d\n",info.conn_info[i].src,\
					  info.conn_info[i].dst,\
					  info.conn_info[i].metric);
	   }
	}
}
//output the table to the topology file
int top_validate(FILE *top){
	int i;
	//index int
	getIP();
	//get ip of current machine
	info.num_serv = NUM_SERVER;
	info.num_conn = Num_conn;
	//set initial numbers
//__________________________________________________________________________________________________//
//____________________________________Reading for server info______________________________________//
	for(i =0; i<info.num_serv; i++){
		fgets(buf,sizeof(buf),top);
		tokens = parseTok(buf);
		//read the line from the topology file  and tokenize it
		if(numTok != 3){
			printf("your topology file has problem\n");
			fclose(top);
			return 0;
		}
		//exit if it has less than 3 tokens
		if((info.serv_info[i].id = atoi(tokens[0]))\
	        && (info.serv_info[i].port=atoi(tokens[2]))){
		//if id and port is not 0, it is in right format
		   if(isValidIP(tokens[1])) strncpy(info.serv_info[i].ip, tokens[1],INET_ADDRSTRLEN);
		   else{
			if(!findIP(tokens[1])){
			   printf("Host | IP in topology is wrong\n");
			   fclose(top);
			   return 0;
			}strncpy(info.serv_info[i].ip,getip,INET_ADDRSTRLEN);
		   }
		   //check if given string is valid ipform, ifnot
		   //convert and save it!
		}else{
			printf("value cannot be 0 | char\n");
			fclose(top);
			return 0;
		}
		//if id or port contains 0 value or string..
		if(strcmp(getip,info.serv_info[i].ip)==0){
			my_id = info.serv_info[i].id;
		}
		//Set my id number in the topology
	}
//_____________________________________________________________________________________________________//
//____________________________________Reading for connection info______________________________________//
	for(i =0; i<info.num_conn; i++){
		fgets(buf,sizeof(buf),top);
		tokens = parseTok(buf);
		//tokenize the following line
		if(numTok!=3){
			printf("your topology file has problem\n");
			fclose(top);
			return 0;
		}
		//test for valid format
		int num1 = atoi(tokens[0]);
		int num2 = atoi(tokens[1]);
		//save temporary src and dst value
		if(my_id != num1 && my_id != num2){
			printf("This topology is not mine..\n");
			fclose(top);
			return 0;
		}if((num1==num2)&&(num1==my_id)){
			printf("Topology cannot contain loopback connection.\nplease review ur topology\n");
			fclose(top);
			return 0;
		}
		//if my_id is not in either src or dst, this is not my topology
		info.conn_info[i].src = my_id;
		//if there exists my id in link, set src my id
		my_id == num1 ? (info.conn_info[i].dst = num2) : (info.conn_info[i].dst = num1);
		//if my_id is same as num1, then dst is num2 or it is num1;
		if((num1!=num2)&&!(info.conn_info[i].metric = atoi(tokens[2]))){
			printf("Cost should be larger than 0!!\n");
			fclose(top);
			return 0;
		}
		//test validity of metric
	}
	fgets(buf,sizeof(buf),top);
	if(!feof(top)){
		printf("Number of connection doesn match with actual connection list\n");
		printf("This was what was remaining %s\n",buf);
		fclose(top);
		return 0;
	}
	//init for 0 to Num_conn	
	for(i =info.num_conn; i<info.num_serv; i++){
		info.conn_info[i].src = my_id;
		info.conn_info[i].metric = inf;
	}
	//givine all infinite value
	for(i=1; i<=info.num_serv; i++){
	//for tracking empty dst
	   int j;
    	   for(j=0; j<info.num_serv; j++){
	   //searching for conn_info.dst,
	    if(info.conn_info[j].dst == i) break;
	    //if dst value is already took place, skip this dst value.
	      if(info.conn_info[j].dst==0){
	      //else i is not used and found empty dst value
		info.conn_info[j].dst = i; 
		//set it with value of i
	        if(info.conn_info[j].src == info.conn_info[j].dst) info.conn_info[j].metric=zero;
		//if connecting myself, give metric zero(0x0000)
		break;
	      }
	   }
	}
	sortConnInfo(info.conn_info);
	//finding src-dst connection not on the topology file
	//set cost inf(0xFFFF) to all connection not shown in topology
	return 1;
}
int main(int argc, char* argv[]){
	if(argc != 5){
		printf(PROC_USAGE);
	//print error if it comes short of number of arguments
	}else{
		if(strcmp("-i",argv[3])==0 && strcmp("-t",argv[1])==0){
		   if(atoi(argv[4])) timeout.tv_sec = atoi(argv[4]);
		   else{
			printf("please give right time interval!\n");
			exit(1);
		   }
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
		  if(NUM_SERVER < Num_conn+1){
			printf("Your number of connection in  topology file is wrong\n");
			fclose(topology);
			exit(1);
		  }
		  //test max connection
		  bzero(&pack, sizeof(pack));
		  bzero(&info, sizeof(info));
		  //init for packet struct 
		  if(top_validate(topology)){
		   //test if topology contains correct data
		  printf("Numserv: %d\nNumCOnn: %d\n",NUM_SERVER ,Num_conn);
		  for(i =0; i< info.num_serv; i++){
			printf("SERVER- id:%d, ip:%s, port:%d\n",info.serv_info[i].id,info.serv_info[i].ip,info.serv_info[i].port);
		  }
		  for(i=0; i< info.num_serv; i++){
			printf("TOPOLOGY- src:%d, dst:%d, metric:%d\n",info.conn_info[i].src,info.conn_info[i].dst,info.conn_info[i].metric);
		  }
		  //testing
		  syncTable(topology);
		  }
		}else{
			printf(PROC_USAGE);
		}
	}
	return 0;
} 
