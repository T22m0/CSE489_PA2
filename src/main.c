#include "../inc/main.h"
//______________________________//
struct timeval timeout;
FILE *topology;
packet pack,pack_income;
sock temp,svSock;
top info,info_check;
fd_set readfds,tempfds;
//__________Main________________//
//________________________________//
char buf[MAX_LINE];
char ans[MAX_INPUT];
char getip[INET_ADDRSTRLEN];
char findip[INET_ADDRSTRLEN];
char findhost[MAX_HOST];
char* command;
char** tokens;
//__________BUFFER______________//
//_______________________________//
int Num_conn, time, packets, update, neighbor;
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
	close(temp.sock);
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
int check_id(int i){
	return (1<=i)&&(i<=info.num_serv);
}
//id should be larger than 1, and less than total number of server
int check_cost(uint16_t i){
	return (zero<i)&&(i<=inf);
}
int check_update(){
	update = 0;
	int i;
	if(info.num_conn != info_check.num_conn) update++;
	//printf("%d update, %d info, %d info_check \n", update,info.num_conn, info_check.num_conn);
	for(i = 0; i < info.num_serv; i++){
		if(info.conn[i].src==info_check.conn[i].dst) continue;
	//printf("%d update, %d info metric, %d info_check metric\n", update, info.conn[i].metric, info_check.conn[i].metric);
		if(info.conn[i].metric != info_check.conn[i].metric) update++;
	}
	return update;
}
//cost should be larger than zero, and less than 0xFFFF
int searchId(char* ip){
	int i;
	for(i =0; i< info.num_serv; i++){
		if(strcmp(info.serv[i].ip,ip)==0) return i;
	}
}
//search/ Id on info.serv from given IP
void sortConnInfo(connection c[]){
	int i,j;
	connection temp;
	for(i=0; i<info.num_serv; i++){	     
	   for(j=i+1; j<info.num_serv; j++){
      	     if(info.conn[i].dst > info.conn[j].dst){
		temp = info.conn[i];
		info.conn[i]=info.conn[j];
		info.conn[j]=temp;	
	     }
	   }  	
	}
}
//sorting conn in order of dst O(N^2) doesn matter in this case because number is small
void printTopology(){
	sortConnInfo(info.conn);
	//sort topology before printout
	int i;
	printf("[Number of servers]: %d\n[Number of Connction]: %d\n\n",info.num_serv ,info.num_conn);
	for(i =0; i< info.num_serv; i++){
		printf("[SERVER%d] id:%d, ip:%s, port:%d\n",i+1,info.serv[i].id,info.serv[i].ip,info.serv[i].port);
	}printf("\n");
	for(i=0; i< info.num_serv; i++){
		printf("[Connection%d] src:%d, dst:%d, metric:%d\n",i+1,info.conn[i].src,info.conn[i].dst,info.conn[i].metric);
	}printf("\n");
	for(i=0; i< info.num_serv; i++){
		printf("[Neighbor%d] %d\n",i+1,info.neig[i]);
	}
	//testing
}
//print out current topology table
void update_table(){
	int i; // just a count
	int sender_id = searchId(findIP(findHost(temp.sock_info)));
	if(!info.neig[sender_id]) info.num_conn++;
	info.neig[sender_id]=1;
	//set neighbor relationship if I get a packet from other server who is not my neighbor
	//Even if I am not neighbor, if you are my neighbor, I know that you are my neighbor
	printf("RECEIVED A MESSAGE FROM SERVER : [%d]\n",sender_id+1);
	//sender id of my list
	    for(i=0;i<info.num_serv;i++){
	    //need to scan all index basically..
		   int tmp_metric = pack_income.info[i].metric;	
		   if(!tmp_metric) continue;
		   // no need to update sender's loopback case
		   temp.sock_info.sin_addr.s_addr = pack_income.info[i].srvip_n;
		   int my_tmp_idn = searchId(inet_ntoa(temp.sock_info.sin_addr));
		   //find id of the ipaddress from received packet. 
		   int tmp_idn = pack_income.info[i].idn;
		   //id from incoming packet 	   
		   if(my_id == tmp_idn){
				info.conn[sender_id].metric=tmp_metric;
		   //if my_id is his tmp_idi syng value with other!
		   }else if(info.conn[my_tmp_idn].metric == inf && tmp_metric < inf){
			if(!info.neig[my_tmp_idn]){
	   			info.conn[my_tmp_idn].metric = info.conn[sender_id].metric + tmp_metric;
				info.num_conn++;
			}
		   //if metric in my table is inf and not in income packet,
		   //and givem id is not my neighbor 
		   //simply update the value now it is up
		   }else if(info.conn[my_tmp_idn].metric <inf && tmp_metric ==inf){
			if(!info.neig[my_tmp_idn]){
				info.conn[my_tmp_idn].metric = inf;
				info.num_conn--;
			}
		   // if my metric is not inf but not in income packet,
		   // and if it is not my neighbor just disconnect it
		   }else if(info.conn[my_tmp_idn].metric < inf && tmp_metric < inf){
			if(!info.neig[my_tmp_idn]) info.conn[my_tmp_idn].metric = info.conn[sender_id].metric + tmp_metric;
			else{
			 	if(info.conn[my_tmp_idn].metric > info.conn[sender_id].metric+tmp_metric) info.conn[my_tmp_idn].metric = info.conn[sender_id].metric + tmp_metric;
			}
		   //if both link is up
		   //we need to compare metric from my table and metric from incomepacket + metric between sender and me
		   //if previous is larger, update!
		   }
		}  
}
int top_validate(FILE *top){
	int i;
	//index int
	getIP();
	//get ip of current machine
	bzero(&info, sizeof(info));
	//init for packet struct 
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
		info.serv[i].id = atoi(tokens[0]);
	        info.serv[i].port=atoi(tokens[2]);
		if(info.serv[i].id && info.serv[i].port){
		//if id and port is not 0, it is in right format
		   if(isValidIP(tokens[1])) strncpy(info.serv[i].ip, tokens[1],INET_ADDRSTRLEN);
		   else{
			if(!findIP(tokens[1])){
		   	   printf("Host | IP in topology is wrong\n");
		      	   fclose(top);
		      	   return 0;
		   	}strncpy(info.serv[i].ip,findip,INET_ADDRSTRLEN);
		   }
		   //check if given string is valid ipform, ifnot
		   //convert and save it!
		}else{
			printf("value cannot be 0 | char\n");
			fclose(top);
			return 0;
		}
		//if id or port contains 0 value or string..
		if(strcmp(getip,info.serv[i].ip)==0){
			my_id = info.serv[i].id;
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
		info.conn[i].src = my_id;
		//if there exists my id in link, set src my id
		my_id == num1 ? (info.conn[i].dst = num2) : (info.conn[i].dst = num1);
		//if my_id is same as num1, then dst is num2 or it is num1;
		info.neig[info.conn[i].dst-1] = 1;
		//record my neighbor routers
		if((num1!=num2)&&!(info.conn[i].metric = atoi(tokens[2]))){
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
		info.conn[i].src = my_id;
		info.conn[i].metric = inf;
	}
	//givine all infinite value
	for(i=1; i<=info.num_serv; i++){
	//for tracking empty dst
	   int j;
    	   for(j=0; j<info.num_serv; j++){
	   //searching for conn.dst,
	    if(info.conn[j].dst == i) break;
	    //if dst value is already took place, skip this dst value.
	      if(info.conn[j].dst==0){
	      //else i is not used and found empty dst value
		info.conn[j].dst = i; 
		//set it with value of i
	        if(info.conn[j].src == info.conn[j].dst) info.conn[j].metric=zero;
		//if connecting myself, give metric zero(0x0000)
		break;
	      }
	   }
	}
	//finding src-dst connection not on the topology file
	//set cost inf(0xFFFF) to all connection not shown in topology
	printf("\n_-_-_-_-_-SUCCESS to parse Topology-_-_-_-_-_\n");
	sleep(1);
	return 1;
}
int init(){
	packets =0;
	update = 0;
	bzero(&pack, sizeof(pack));
	bzero(&svSock,sizeof(svSock));
	FD_ZERO(&readfds);
	//initialize all necessary variables
	// count of packets, update, zeroing -[packet structure, socket structure and fd_set]
	if((svSock.sock = socket(AF_INET,SOCK_DGRAM,0))==-1){
		perror("ERROR! CANT create socket:");
		return 0;
	}
	//make udp socket
	svSock.sock_info.sin_family = AF_INET;
	svSock.sock_info.sin_port = htons(info.serv[my_id-1].port);
	svSock.sock_info.sin_addr.s_addr= inet_addr(info.serv[my_id-1].ip);
	//printf("%s is my ip %d my port\n",info.serv[my_id-1].ip,info.serv[my_id-1].port);
	if((bind(svSock.sock,(struct sockaddr*)&svSock.sock_info, sock_len))==-1){
		perror("ERROR! Cant bind:");
		return 0;
	}
	//bind the socket	
	timeout.tv_sec = MAX_WAIT;
	if(setsockopt(svSock.sock,SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout))<0){
		perror("socket setting error! ");
		return 0;
	}
	//setting socket to wait on receiving for MAX_WAIT seconds - 3 seconds in this case
	timeout.tv_sec = time;
	//setting time out for step() method
	printf("\n_-_-_-_-_-SUCCESS to Bind socket-_-_-_-_-_\n");
	sleep(1);
	return 1;
}
//initialize and setting and bind socket
void makePacket(){
	int i;
	pack.num_update = check_update();
	pack.srv_port = info.serv[my_id-1].port;
	pack.srv_ip= inet_addr(info.serv[my_id-1].ip);
	//printf("%d update \n%d server port\n%d server IP\n",update,pack.srv_port, pack.srv_ip);
	for(i = 0; i < info.num_serv; i++){
		pack.info[i].srvip_n= inet_addr(info.serv[i].ip);
		pack.info[i].srvportn= info.serv[i].port;
	      //pack.info[i]pad = do nothing
	        pack.info[i].idn = info.serv[i].id;
		pack.info[i].metric= info.conn[i].metric;
		//printf("\n%dTh info %d-server ip %d-server port %d-server pad %d-server id %d-server metric \n",i,pack.info[i].srvip_n,pack.info[i].srvportn,pack.info[i].pad,pack.info[i].idn,pack.info[i].metric);
	}
	printf("\nSuccess to make a packet\n");
}

int step(){
	int i;
	neighbor=0;
	//init neighbor to 0
	makePacket();
	//make packet
	for(i =0; i< info.num_serv; i++){
	   if(info.neig[i]){	
	   //search my neighbors to send the packet
		temp.sock_info.sin_port = htons(info.serv[i].port);
		temp.sock_info.sin_addr.s_addr=inet_addr(info.serv[i].ip);
		//set info of neighbors to temp socket
		printf("__________________________________________\n");
		//printf("gonna send the packet\n");
		sendto(svSock.sock, &pack, sizeof(pack),0,(struct sockaddr*) &temp.sock_info, sock_len);
		//printf("now trying receiving..\n");
		ans[0] = '\0';
		recvfrom(svSock.sock,ans,sizeof("receive"),0,(struct sockaddr*)&temp.sock_info,&sock_len);
		if(strcmp(ans,"receive")==0){
		//if host is alive, I would get the receive the answer from the other server
			neighbor++;
			//so record the that i succefully send the packet
			info.neig[i] = 1;
			//now i know my neighbor is alive!
			//printf("recived from server : %s with port :%d \n",inet_ntoa(temp.sock_info.sin_addr),ntohs(temp.sock_info.sin_port));
			printf("Successfully Send The Packet to Server : [%d]\n", searchId(inet_ntoa(temp.sock_info.sin_addr))+1);
		}else{
			info.neig[i]++;
			//record if I did not get the message from the server
			printf("failed to receive from server ID [%d]\n", searchId(inet_ntoa(temp.sock_info.sin_addr))+1);
			if(info.neig[i] == 4){
				info.conn[i].metric = inf;
				info.num_conn--;
				info.neig[i] = 0;
				printf("It seems server[%d] is dead..The cost will be set to infinity\n",i+1);
			}	
			//if it is 3rd time that i missed the packet, 
			//consider it is dead now, and set metric to inf
		}
		//and send the data through my socket
		printf("__________________________________________\n");
	   }
	}
	info_check = info;
	timeout.tv_sec = time;
	//since I updated packet and successfully send it to other neighbors, 
	//update the info packet
	//then renew my info
	if(!neighbor) return 0;	
	else return 1;
}
//method to send the packet to only neighbors
void runServ(){
	int fd_count;
	FD_SET(0,&readfds);
	FD_SET(svSock.sock,&readfds);
	//setting basic fd_set setting
	info_check = info;
	//save current info table.
 	//saved table will be used to check update!
	while(1){
	   fprintf(stderr,"PA2>");
	   //printout terminal
	   tempfds = readfds;
	   //renew fd_set
	   fd_count = select(svSock.sock+1,&tempfds,NULL,NULL,&timeout);
	   //if incoming data or keyboard input execute below!
	   if(!fd_count){
		if(step()) printf("packet broadcasted\n)");
		else printf("no neighbor is up!\n");
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
				   if((num1==my_id)&&(num2!=my_id)){
				   // num1 should my_id and num2 should not be my id.
				   // if num1==num2, its loop back
				   // if num1 = neighbor,  & num2 =my_id, its wrong input
				     if(info.neig[num2-1]){
				      // check if given ID is my neighbor
					if(info.conn[num2-1].metric != metric){
					// check if cost for update is same as current costi
					   info.conn[num2-1].metric = metric;
					   //then update
					   if(metric == inf){
					   // bur if you are updating cost to neighbor to infinity, 
						 info.num_conn--;
						 info.neig[num2-1]=0;
						//it is no more your neighbor,
					   }
					   printf("<%s> SUCCESS!\n",tokens[0]);
					}else printf("<%s> The cost is already set to the value! no need to update\n",tokens[0]);
				      }else printf("<%s> Given Machine is not my neighbor!\n",tokens[0]);
				   }else printf("<%s> First Argument should be My id and second neighbor!!\n",tokens[0]);
				}else printf("<%s> Please check arguments for update!\n",tokens[0]);
			}else if((strcasecmp(tokens[0],"step")==0)&&numTok==1){
				if(step()) printf("<%s> SUCCESS\n",tokens[0]);
				else printf("<%s> No neighbor is up near me..\n",tokens[0]);
			}else if((strcasecmp(tokens[0],"packets")==0)&&numTok==1){
				printf("I got %d packets from the last time packets command executed\n",packets);
				printf("<%s> SUCCESS\n",tokens[0]);
				packets = 0;
			}else if((strcasecmp(tokens[0],"display")==0)&&numTok==1){
				printTopology();
				printf("<%s> SUCCESS!\n",tokens[0]);
			}else if((strcasecmp(tokens[0],"disable")==0)&&numTok==2){
				int dst = atoi(tokens[1]);
				//parse given input 
				if(dst && dst <= NUM_SERVER && dst != info.conn[dst-1].src){
				//to update cost value of given server id given dst id should be
				//	1. dst should be larger than 0
				//	2. dst should be equal or less than total number of server
				//	3. dst should not be same as src - cannot update loopback
				    if(info.neig[dst-1]){
					if(info.conn[dst-1].metric != inf){
					// if connection is already disabled, skip changing value
					    info.conn[dst-1].metric = inf;
					    info.neig[dst-1] = 0;
					    info.num_conn--;
					    printf("<%s> SUCCESS\n",tokens[0]);
				        } else printf("<%s> connection to given server ID is already disabled\n",tokens[0]);
				    }else printf("<%s> you are trying to disable who is not your neighbor!!\n",tokens[0]);
				}else printf("<%s> server ID is not valie\n",tokens[0]);
			}else if((strcasecmp(tokens[0],"crash")==0)&&numTok==1){
				int i;
				close(svSock.sock);
				FD_CLR(svSock.sock,&readfds);
				//disable my only socket.. cannot receive or send any data from now
				printf("Closed Datagram socket\n");	
				info.num_conn=0;
				for(i = 0; i<info.num_serv; i++){
					info.conn[i].metric = inf;
					info.neig[i] = zero;
				}
				//wipe out all neighbor data
				printf("Disconnected all connction\n");
				printTopology();
				printf("<%s> SUCCESS\n",tokens[0]);
			}else printf(USAGE);
//________________________________________________________________________________________________________//
//_______________________________Incoming packet_________________________________________________________//
		   }else if(FD_ISSET(svSock.sock,&tempfds)){
			printf("\n****************************************\n");
			if(recvfrom(svSock.sock, &pack_income, sizeof(pack_income),0,(struct sockaddr*)&temp.sock_info,&sock_len)>0) packets++;
			sendto(svSock.sock,"receive",sizeof("receive"),0,(struct sockaddr*)&temp.sock_info,sock_len);
			update_table();
			printf("***************************************\n");
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
