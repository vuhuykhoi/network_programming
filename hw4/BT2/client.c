#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "filehelper.h"


#define BUFF_SIZE 8192


int main(int argc ,char *argv[])
{
	char line[LINELENGTH+1];
	int msg_len, bytes_sent, bytes_received;
	int client_sock = 0 ;
	char buff[BUFF_SIZE+1],fURL[FILEURLLENGTH+1];
	struct sockaddr_in server_addr; /* server's address information */
	char SERVER_ADDR[20];
	FILE *fin;

	if(argc != 3){
		printf("Invalid argument\n");
		exit(1);
	}
	//get serer port and server addr from argument
	int SERVER_PORT = atoi(argv[2]);
	strcpy(SERVER_ADDR,argv[1]);
	
	//CLIENT CONNECT and CLOSE to SERVER again and again!!!!
	while(1){
		//Step 1: Construct socket
		client_sock = socket(AF_INET,SOCK_STREAM,0);
		
		//Step 2: Specify server address
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(SERVER_PORT);
		server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
		
		//Step 3: Request to connect server
		if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
			printf("\nError!Can not connect to sever! Client exit imediately! ");
			close(client_sock);
			return 0;
		}
		
		//Step 4: Communicate with server			
		
		printf("\nInsert file to upload:");
		memset(fURL,'\0',(strlen(fURL)+1));
		fgets(fURL, BUFF_SIZE, stdin);		
		fURL[strlen(fURL)-1] = '\0';
		msg_len = strlen(fURL);
		if (msg_len == 0){
			
			break;
		} 
			
		//check if file existed ,if existed ->SEND
		if(!isExsistedFile(fURL)){
			printf("Error: File not found\n");
			close(client_sock);
			continue;
		}
		//open file to read
		fin = fopen(fURL,"rb");
		if(!fin){
	 		printf("Error: File not found\n");
			
			continue;
	 	}

	 	//SEND: filename and file format to SERVER
		bytes_sent = send(client_sock, fURL, msg_len, 0);
		if(bytes_sent <= 0){
			printf("\nConnection closed!\n");
			continue;
		}
			
		//RECEIVE: echo message:if file is existed on server?
		bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
		if(bytes_received < 0){
			printf("\nError!Cannot receive data from sever!\n");
			//break;
			continue;
		}
			
		//if file is existed on server cannot upload
		buff[bytes_received] = '\0';
		
		if(!strcmp(buff,"Error: File is existent on server")){
			printf("%s\n", buff);
		//if file is not existed on server start uploading file!
		}else{
			//SEND :file content
			bzero(line, LINELENGTH);
			//////////////////////////////////////////////////////
			//send bytes MUST equal fread return byte!!!!!!!!!!!//
			//////////////////////////////////////////////////////
				
			while(bytes_sent = fread(line,sizeof(char),LINELENGTH,fin)){
				if(send(client_sock, line, bytes_sent, 0) < 0 ){
					printf("Error: File tranfering is interupted\n");
			        break;
				}
				bzero(line, LINELENGTH);
			}
			printf("Successful transfering\n");
			fclose(fin);
			
		}
		close(client_sock);
		
	}
	return 0;
}
