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
#include <random>

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
	double latency;
	int NumFiles;
	string type;
	sockaddr_in server_addr;

};

void recv_file(int sockfd);
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
	/**/
	

	/* Creating 'NumThr' number of clients*/
	for(int i=0;i<NumThr;i++){
		td[i].server_addr = server_addr;
		td[i].total_time = total_time;
		td[i].sleep_time = sleep_time;
		td[i].type = type;
		pthread_create(&threads[i], NULL,  getFile, (void *)&td[i]);

	}


	Timer file_request;
	file_request.reset();
	for(int i=0;i<NumThr;i++){
		pthread_join(threads[i], NULL);
	}
	double Exp_Time;
	Exp_Time=file_request.elapsed();

	cout<<"Done"<<endl;


	double tot_latency=0, totNum_files=0;
	for(int i=0;i<NumThr;i++){
		tot_latency+= td[i].latency;
		totNum_files+= td[i].NumFiles;
	}

	double avg_throughput = totNum_files/Exp_Time;
	cout<<"throughput = "<<avg_throughput<<" req/s"<<endl;

	double avg_latency=tot_latency/NumThr;
	cout<<"average response time = "<<avg_latency<<" sec"<<endl;


	return 0;
}

void recv_file(int sockfd){
	
}

void *getFile(void *thread_arg){
	thread_data *data;
	data = (struct thread_data *) thread_arg;
	string message;
	int i;
	Timer timer;
	double tim, response_time;
	int Round=0;
	timer.reset();

	std::random_device rd;     // only used once to initialise (seed) engine
	std::mt19937 gen(rd());    // random-number engine used (Mersenne-Twister in this case)
	std::uniform_int_distribution<int> dis(0, 9999); // guaranteed unbiased

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

		if(data->type == "random"){
			i= dis(gen);
		}
		else{
			if(data->type == "fixed"){
				i=0;
			}
			else{
				perror("type not defined");
				exit(0);
			}
		}
		message = "get files/foo"+to_string(i)+".txt";
		cout<<message<<endl;break;
		const char *mes;
		mes = message.c_str();
		send(sockfd, mes, message.length(), 0);
		recv_file(sockfd);
		if(close(sockfd)<0){
			perror("socket close error");
			exit(EXIT_FAILURE);
		}
		file_time = file_timer.elapsed();
		response_time += file_time;
		Round++;
		tim = timer.elapsed();
		if(tim>=data->total_time){	
			data->NumFiles = Round;
			break;
		}
	}
	data->latency = response_time/Round;
	pthread_exit(NULL);
}
