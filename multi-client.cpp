#include <iostream>
#include <string.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <pthread.h>

using namespace std;

struct  thread_data
{
	int sockfd, thread_id;
	double latency, throughput;
	bool sleep=0;
};

void *getFile(void *sockfd);

int main(int argc, char *argv[]){

	int port_no, NumThr;
	int * sockfd;

	string ip;

	in_addr *HW_addr;

	pthread_t *threads;

	struct hostent *server;
	struct sockaddr_in server_addr;
	string type;

	thread_data *td;


	/* check for correct number of arguments */
	if(argc != 7){
		printf("usage %s IP port threads duration think_time mode(random or fixed)\n", argv[0]);
	}


	/* Get port number from the arguments and convert it to unsigned long */
	port_no=stoul(argv[2], nullptr, 0);


	/* Number of threads that should be created*/
	NumThr = stoul(argv[3], nullptr, 0);

	type = argv[6];

	/* Declaring an array of socke file descriptors, one for each thread*/
	//sockfd = new int[NumThr];

	/* Deaclaring 'NumThr' size pthread_t  array*/
	threads = new pthread_t[NumThr];

	/* Declaring an array of type thread_data, to pass as argument to function in thread_create*/
	td = new thread_data[NumThr];


	/* Get host-server from 'ip or hostname' and fill the server address in sockaddr_in data structure*/
	server = gethostbyname(argv[1]);
	if(server == NULL){
		cout<<"Error: No such host\n";
		return 0;
	}
	HW_addr = (in_addr*) server->h_addr;
	ip = inet_ntoa(* HW_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_no);			// port number is converted from 'host' to 'network' short
	server_addr.sin_addr.s_addr = inet_addr(ip.c_str());	

	/*

	Need clarification	*/
	memset(&(server_addr.sin_zero), '\0', 8);
		

	/*/


	/* Connect to server_addr 'NumThr' times by creating 'NumThr' threads*/
	for(int i=0;i<NumThr;i++){

		/* Wait until server is connected to a client*/
		while(true){

			/* Creating a socket of type stream socket */
			td[i].sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(td[i].sockfd<0){
				cout<<"Error opening socket\n";
				return 0;
			}

			/* trying to connect to the server */
			int res = connect(td[i].sockfd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr));
			if(res == 0){  // Connected

				td[i].thread_id = i;

				/* Creating a thread for a client by passing sockfd[i] as parameter, so that it can communicate with server independently*/
				pthread_create(&threads[i], NULL,  getFile, (void *)&td[i]);

				/* One client connected to server, exit the while loop and create another client */
				break;
			}
			else{

			}
		}
		
	}

	/*int active=NumThr;
	while(active>0){
		for(int i=0;i<NumThr;i++){
			if(td[i].active)
			if(td[i].sleep==1){
				td[i].sockfd = socket(AF_INET, SOCK_STREAM, 0);
				if(td[i].sockfd<0){
					cout<<"Error opening socket\n";
					return 0;
				}
				int res = connect(td[i].sockfd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr));
				if(res == 0){ // connected
					td[i].sleep=0;
				}
			}
			
		}
	}*/


	for(int i=0;i<NumThr;i++){
		pthread_join(threads[i], NULL);
	}

	return 0;
}

void *getFile(void *thread_arg){
	thread_data *data;
	data = (struct thread_data *) thread_arg;
	int sockfd = data->sockfd;
	int thread_id = data->thread_id;
	string message;

	while(true){	
		int i=0;
		message = "get files/foo"+to_string(i)+".txt";
		cout<<message<<endl;break;
		const char *mes;
		mes = message.c_str();
		send(sockfd, mes, message.length(), 0);
		//recv(sockfd, , 512, 0)
		/* Need editing */
	}
	pthread_exit(NULL);
}
