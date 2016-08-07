#include <iostream>
#include <string.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <pthread.h>
#include <chrono>
#include <unistd.h>

using namespace std;

class Timer
{
public:
    Timer() : beg_(clock_::now()) {}
    void reset() { beg_ = clock_::now(); }
    double elapsed() const { 
        return std::chrono::duration_cast<second_>
            (clock_::now() - beg_).count(); }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> beg_;
};

struct  thread_data
{
	double total_time, sleep_time;
	double latency, throughput;
	sockaddr_in server_addr;

};

void *getFile(void *sockfd);

int main(int argc, char *argv[]){

	int port_no, NumThr;
	int * sockfd;

	double total_time, sleep_time;

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

	total_time = atoi(argv[4]);

	sleep_time = atoi(argv[5]);

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


	/* Creating 'NumThr' number of clients*/
	for(int i=0;i<NumThr;i++){
		td[i].server_addr = server_addr;
		td[i].total_time = total_time;
		td[i].sleep_time = sleep_time;
		pthread_create(&threads[i], NULL,  getFile, (void *)&td[i]);

	}

	for(int i=0;i<NumThr;i++){
		pthread_join(threads[i], NULL);
	}

	return 0;
}


void *getFile(void *thread_arg){
	thread_data *data;
	data = (struct thread_data *) thread_arg;
	string message;

	Timer timer;
	double tim, response_time;
	int round=0;
	timer.reset();
	while(true){	

		/* Open a socket and connect to the server*/
		Timer file_timer;
		double file_time;
		file_timer.reset();
		int sockfd=socket(PF_INET, SOCK_STREAM, 0);
		while(true){
			cout<<data->sleep_time<<endl;
			int res = connect(sockfd, (struct sockaddr *) &data->server_addr, sizeof(struct sockaddr));
			if(res == 0){
				break;
			}
		}

		int i=0;
		message = "get files/foo"+to_string(i)+".txt";
		cout<<message<<endl;break;
		const char *mes;
		mes = message.c_str();
		send(sockfd, mes, message.length(), 0);
		//recv(sockfd, , 512, 0)
		/* Need editing */
		close(sockfd);
		file_time = file_timer.elapsed();
		response_time += file_time;
		round++;
		tim = timer.elapsed();
		if(tim>=data->total_time){
			break;
		}
	}
	data->latency = response_time/round;
	pthread_exit(NULL);
}
