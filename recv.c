#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001
#define MAX_LEN 1396

static  int count = 0;

void receive_file(msg r){
	int sum = 0;
	int size;
	int returned = 0;
  if(recv_message(&r)<0){
      perror("Receive message");
      exit(0);
  }
  
  for(int i = 0; i < MAX_LEN; i++)
  	for(int j = 0; j < 8; j++){
  		sum = sum ^ ((1<<j) & r.payload[i])	; 
  	}

  if(r.checksum != sum){
  	printf("[Error] checksum, adding one\n");
  	count++;
  }

  int file = open(r.payload, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  sprintf(r.payload,"%s", "ACK");
  r.len = strlen(r.payload) + 1;
  send_message(&r);
  printf("[recv] ACK sentt\n");

  sum = 0;
  if(recv_message(&r)<0){
      perror("Receive message");
      exit(0);
  }
  
  for(int i = 0; i < MAX_LEN; i++)
  	for(int j = 0; j < 8; j++){
  		sum = sum ^ ((1<<j) & r.payload[i]); 
  	}

  if(r.checksum != sum){
  	printf("[Errorr] checksum, adding one\n");
  	count++;
  }

  size = atoi(r.payload);

  sprintf(r.payload,"%s", "ACK");
  r.len = strlen(r.payload) + 1;
  send_message(&r);
  printf("[recv] ACK sent\n");

  for(;returned < size; returned += r.len){
  	sum = 0;
  	if(recv_message(&r)<0){
    	perror("Receive message");
      	exit(0);
  	}
  
  	for(int i = 0; i < MAX_LEN; i++)
  		for(int j = 0; j < 8; j++){
  			sum = sum ^ ((1<<j) & r.payload[i]);
  		}

  	if(r.checksum != sum){
  		printf("[Error] checksum, adding one\n");
  		count++;
  	}
  	write(file, r.payload, r.len);

  	sprintf(r.payload,"%s", "ACK");
  	r.len = strlen(r.payload) + 1;
  	send_message(&r);
  	printf("[recv] ACK sent\n");
  }
  close(file);
}

int main(void)
{
	msg r;
	int i, res;
	
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);
	
	for (i = 0; i < COUNT; i++) {
		/* wait for message */
		res = recv_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			return -1;
		}
		
		/* send dummy ACK */
		res = send_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}
	}

	receive_file(r);
	printf("corrupted %d\n", count);
	printf("[RECEIVER] Finished receiving..\n");
	return 0;
}
