#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000
#define MAX_LEN 1396

void send_file(char *fileName, int number){
	msg t;
	int sum, res, size, sent_size;
	int file = open(fileName, O_RDONLY);
	sprintf(t.payload, "%s-copy", fileName);
	t.len = strlen(t.payload)+1;
	sum = 0;
	for(int i = 0; i < MAX_LEN; i++)
  		for(int j = 0; j < 8; j++){
  			sum = sum ^ ((1<<j) & t.payload[i]);
  		}
	t.checksum = sum;
	send_message(&t);

	res = recv_message(&t);
	if (res < 0) {
		perror("[SENDER] Receive error. Exiting.\n");
		exit(0);
	} else {
		printf("[Sent] Got reply\n");
	}
	size = lseek(file, 0, SEEK_END);
	sprintf(t.payload, "%d", size);
	t.len = strlen(t.payload) + 1;
	sum = 0;
	for(int i = 0; i < MAX_LEN; i++)
  		for(int j = 0; j < 8; j++){
  			sum = sum ^ ((1<<j) & t.payload[i]);
  		}
	t.checksum = sum;
	send_message(&t);
	res = recv_message(&t);
	if (res < 0) {
		perror("[SENDER] Receive error. Exiting.\n");
		exit(0);
	} else {
		printf("[Sent] Got reply\n");
	}
	int file2;
	for(int i = 0; i < (number * 100) / (8* size);i++)
	{
		file2 = read(file, t.payload, MAX_LEN);
		if(file2 < 0){
			printf("Reading error\n");
			exit(0);
		}
		t.len = file2;
		sent_size +=file2;
		sum = 0;
		for(int i = 0; i < MAX_LEN; i++)
  		for(int j = 0; j < 8; j++){
  			sum = sum ^ ((1<<j) & t.payload[i]);
  		}
		t.checksum = sum;
		send_message(&t);
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			exit(0);
		} else {
			printf("[Sent] Got reply\n");
		}
		
	}
	while(sent_size < size){
		file2 = read(file, t.payload, MAX_LEN);
		if(file2 < 0){
			printf("Reading error\n");
			exit(0);
		}
		t.len = file2;
		sum = 0;
		for(int i = 0; i < MAX_LEN; i++)
  		for(int j = 0; j < 8; j++){
  			sum = sum ^ ((1<<j) & t.payload[i]);
  		}
		t.checksum = sum;
		send_message(&t);
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			exit(0);
		} else {
			printf("[Sent] Got reply\n");
		}
		sent_size += size;
	}
	for(int i = 0; i < (number * 100) / (8* size);i++)
	{
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			exit(0);
		} else {
			printf("[Sent] Got reply\n");
		}
	}
	close(file2);
	close(file);
}

int main(int argc, char *argv[])
{
	msg t;
	int i, res;
	
	printf("[SENDER] Starting.\n");	
	init(HOST, PORT);

	/* printf("[SENDER]: BDP=%d\n", atoi(argv[1])); */
	
	for (i = 0; i < COUNT; i++) {
		/* cleanup msg */
		memset(&t, 0, sizeof(msg));
		
		/* gonna send an empty msg */
		t.len = MSGSIZE;
		
		/* send msg */
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
		
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
	}
	send_file("file.in", atoi(argv[1]));
	printf("[SENDER] Job done, all sent.\n");
	
	return 0;
}
