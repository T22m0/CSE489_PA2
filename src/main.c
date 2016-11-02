#include "../inc/main.h"
//______________________________//
struct timeval timeout;
FILE *topology;
packet pack;
sock temp,svSock;
top info;
fd_set readfds,tempfds;
//__________Main________________//
//________________________________//
char buf[MAX_LINE];
char getip[INET_ADDRSTRLEN];
char findip[INET_ADDRSTRLEN];
char findhost[MAX_HOST];
char* command;
char** tokens;
//__________BUFFER______________//
//_______________________________//
int Num_conn;
int time;
int packets;
unsigned int numTok;
int my_id=0;
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
		sprintf(findip,"NA");
		return findip;
	}
	memcpy(&temp.sock_info.sin_addr, h->h_addr_list[0], h->h_length);
	inet_ntop(AF_INET, &temp.sock_info.sin_addr, findip, INET_ADDRSTRLEN);
	return findip;
}
// will return hostname from IP address and save to getip
char* findHost(struct sockaddr_in addr){
	struct sockaddr_in t_addr = addr;
	getnameinfo((struct sockaddr*)&t_addr, sock_len, findhost, sizeof(findhost),NULL,0,0);
	return findhost;
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
void printTopology(){
	sortConnInfo(info.conn_info);
	//sort topology before printout
	int i;
	printf("[Number of servers]: %d\n[Number of Connction]: %d\n",info.num_serv ,info.num_conn);
	for(i =0; i< info.num_serv; i++){
		printf("[SERVER] id:%d, ip:%s, port:%d\n",info.serv_info[i].id,info.serv_info[i].ip,info.serv_info[i].port);
	}
	for(i=0; i< info.num_serv; i++){
		printf("[Connection] src:%d, dst:%d, metric:%d\n",info.conn_info[i].src,info.conn_info[i].dst,info.conn_info[i].metric);
	}
	//testing
}
//print out current topology table
/*void syncTable(FILE *top){
	int i;
	rewind(top);
	//fseek(top,0,SEEK_SET);
	//heading beginning of the file
	fprintf(top,"%d\n%d\n",info.num_serv, info.num_conn);
	printf("%d is current num_conn\n",info.num_conn);
	for(i =0; i<info.num_serv;i++){
		fprintf(top, "%d %s %d\n",info.serv_info[i].id,\
					  info.serv_info[i].ip,\
					  info.serv_info[i].port);
	}
	for(i =1; i<info.num_serv;i++){
	   if(info.conn_info[i].metric != inf){
		fprintf(top, "%d %d %d\n",info.conn_info[i].src,\
					  info.conn_info[i].dst,\
					  info.conn_info[i].metric);
	   } 
	}
	fprintf(top,"SUCCESS To doso\n");
}*/ 
//output the table to the topology file
//no need this function...
int check_id(int i){
	return (1<=i)&&(i<=info.num_serv);
}
//id should be larger than 1, and less than total number of server
int check_cost(uint16_t i){
	return (zero<i)&&(i<=inf);
}
//cost should be larger than zero, and less than 0xFFFF
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
		   	}strncpy(info.serv_info[i].ip,findip,INET_ADDRSTRLEN);
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
	if(!my_id){
		printf("My id is not on the topology.. give me another topology\n");
		fclose(top);
		return 0;
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
		// cost of connection except loopback should be larger than 0!
			printf("Cost should be larger than 0!!\n");
			fclose(top);
			return 0;
		}
		//test validity of metric
	}
	fgets(buf,sizeof(buf),top);
	if(!feof(top)){
		printf("Number of connection doesn match with actual connection list\n");
		printf("This was what was remaining in the file--> %s\n",buf);
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
	//finding src-dst connection not on the topology file
	//set cost inf(0xFFFF) to all connection not shown in topology
	printf("\n_-_-_-_-_-SUCCESS to parse Topology-_-_-_-_-_\n");
	sleep(2);
	return 1;
}
int init(){
	packets =0;
	//initialize packets variable
	bzero(&svSock,sizeof(svSock));
	if((svSock.sock = socket(AF_INET,SOCK_DGRAM,0))==-1){
		perror("ERROR! CANT create socket:");
		return 0;
	}
	svSock.sock_info.sin_family = AF_INET;
	svSock.sock_info.sin_port = htons(info.serv_info[my_id-1].port);
	svSock.sock_info.sin_addr.s_addr= inet_addr(info.serv_info[my_id-1].ip);
//	printf("%s is my ip %d my port\n",info.serv_info[my_id-1].ip,info.serv_info[my_id-1].port);
	if((bind(svSock.sock,(struct sockaddr*)&svSock.sock_info, sock_len))==-1){
		perror("ERROR! Cant bind:");
		return 0;
	}	
	printf("\n_-_-_-_-_-SUCCESS to Bind socket__-_-_-_-_\n");
	sleep(2);
	return 1;
}
void runServ(){
	int fd_count;
	FD_SET(0,&readfds);
	FD_SET(svSock.sock,&readfds);
	//setting basic fd_set setting
	while(1){
	   fprintf(stderr,"PA2>");
	   //printout terminal
	   tempfds = readfds;
	   timeout.tv_sec = time;
	   //renew time clock! && renew fd_set
	   fd_count = select(svSock.sock+1,&tempfds,NULL,NULL,&timeout);
	   //if incoming data or keyboard input execute below!
	   
	   if(!fd_count){
		 printf("send packets\n");
	   }else{
		while(fd_count-- > 0){
//__________________________________________________________________________________________________________//
//_______________________________keyboard input____________________________________________________________//
		   if(FD_ISSET(0,&tempfds)){
			command = readLine();
			tokens = parseTok(command);
			if(tokens[0]=='\0') continue;
			if((strcasecmp(tokens[0],"update")==0)&&numTok==4){
				int num1 = atoi(tokens[1]);
				int num2 = atoi(tokens[2]);
				//parse ids from tokens
				int metric;
				strcasecmp(tokens[3],"inf")==0 ? (metric=inf) : strcasecmp(tokens[3],"zero")==0 ? (metric=zero): (metric=atoi(tokens[3]));						//metric could be either 'inf', 'zero' or any numeric value between 0<x<65535
				if(check_id(num1)&&check_id(num2)&&check_cost(metric)){
				//check if all arguments are valid
			   	   if((num1==my_id)||(num2==my_id)){
				   // check if the user tries to change cost of my connection
				      if(num1!=num2){
				      // check if the usier tries to change cost for looopback
				        int cur_cost;
					my_id == num1 ? (cur_cost = info.conn_info[num2-1].metric) : (cur_cost = info.conn_info[num1-1].metric);
					// save cost value for update-to-same-cost case
					if(cur_cost != metric){
					// check if cost for update is same as current cost
					   my_id == num1 ? (info.conn_info[num2-1].metric=metric) : (info.conn_info[num1-1].metric=metric);
				   	   metric == inf ? info.num_conn-- : info.num_conn++;
					   // if metric is inf, it is the case same as disable. or it is enabling connection between two hosts
					   printf("<%s> SUCCESS!\n",tokens[0]);
					}else printf("<%s> The cost is already set to the value! no need to update\n",tokens[0]);
				      }else printf("<%s> you cannot update cost for loopback!\n",tokens[0]);
				   }else printf("<%s> Your arguments does not contain id of current machine!\n",tokens[0]);					
				}else printf("<%s> Please check arguments for update!\n",tokens[0]);
			}else if((strcasecmp(tokens[0],"step")==0)&&numTok==1){
			}else if((strcasecmp(tokens[0],"packets")==0)&&numTok==1){
				packets = 0;
			}else if((strcasecmp(tokens[0],"display")==0)&&numTok==1){
				printTopology();
				printf("<%s> SUCCESS!\n",tokens[0]);
			}else if((strcasecmp(tokens[0],"disable")==0)&&numTok==2){
				int dst = atoi(tokens[1]);
				//parse given input 
				if(dst && dst < NUM_SERVER && dst != info.conn_info[dst-1].src){
				//to update cost value of given server id given dst id should be
				//	1. dst should be larger than 0
				//	2. dst should be less than total number of server
				//	3. dst should not be same as src - cannot update loopback
				    if(info.conn_info[dst-1].metric != inf){
				    // if connection is already disabled, skip changing value
					info.conn_info[dst-1].metric = inf;
					info.num_conn--;
					printf("<%s> SUCCESS\n",tokens[0]);
				    }
				    else printf("<%s> connection to given server ID is already disabled\n",tokens[0]);
				}else{
				   printf("<%s> server ID is not valie\n",tokens[0]);
				}
			}else if((strcasecmp(tokens[0],"crash")==0)&&numTok==1){
			}else printf(USAGE);
//________________________________________________________________________________________________________//
//_______________________________Incoming packet_________________________________________________________//
		   }else if(FD_ISSET(svSock.sock,&tempfds)){
//________________________________________________________________________________________________________//
//_______________________________COMPROMISED-impossible case______________________________________________//
		   }else{
			printf("I am under attack! HELP!!\n");	
		   }
		}
	   }
	}
}
int main(int argc, char* argv[]){
	if(argc != 5){
		printf(PROC_USAGE);
	//print error if it comes short of number of arguments
	}else{
  	   if(strcmp("-i",argv[3])==0 && strcmp("-t",argv[1])==0){
	      if(atoi(argv[4])) time = atoi(argv[4]);
	      else{
		 printf("please give right time interval!\n");
		 exit(1);
	      }
	      //test interval validity
	      topology = fopen(argv[2],"r");	
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
	      FD_ZERO(&readfds);
	      //init for packet struct 
	      if(top_validate(topology) && init()){
	      //test if topology contains correct data
	 	 if(fclose(topology)!=0){
			perror("cannot close");
			exit(1);
		 }
		 //no more need the topology
		 printTopology();
		 //print beginning result 
		 runServ();
		 //run routing.
	      }else printf("Please check ur topology file\n");	
	   }else{
		printf(PROC_USAGE);
	   }
	}
	return 0;
} 
