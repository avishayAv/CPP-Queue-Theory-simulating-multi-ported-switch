#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <queue>
#include <random>
#include <cmath>
#include <time.h>
#include <iomanip>

class package{
public:
    double time;
    package(double t):time(t){}
};

bool operator<(const package& pac1, const package& pac2){
    return pac1.time < pac2.time;
}

class exit_port{
public:
    std::queue<package> waiting;
    long int waiting_size;
    int rate;
    long int success_messages;
    long int thrown_messages;
    double service_time;
    double total_service_time;
    double total_wait_time;
    long int queue_packages_arrived;
    
    exit_port(int waiting_size, double rate):waiting_size(waiting_size-1),rate(rate),success_messages(0),
            thrown_messages(0),service_time(0),total_service_time(0),total_wait_time(0),
            queue_packages_arrived(0){}
    
    void simulation(){
        std::mt19937 generator(time(0));//random num generator
        std::exponential_distribution<double> message_time(rate);//l*e^(-l*x)
        while(!waiting.empty()){//should we add something about the time ???
            package current_pac = waiting.front();
            waiting.pop();
            if(service_time <= current_pac.time){
                //package arrived after we finished serving - should serve
                success_messages++;
                if(queue_packages_arrived == 0){
                    //message being served immediatly when no other waiting
                    double cur_service_time = message_time(generator);
                    service_time = current_pac.time + cur_service_time;
                    total_service_time+= cur_service_time;
                }
                else{
                    //a message still exists in the queue
                    double temp_time = service_time;
                    while(temp_time <= current_pac.time && queue_packages_arrived > 0){
                        //message can't be served yet, simulate other messages being served
                        double cur_service_time = message_time(generator);
                        temp_time+= cur_service_time;
                        total_service_time+= cur_service_time;
                        queue_packages_arrived--;
                        total_wait_time+= cur_service_time*queue_packages_arrived;
                    }
                    if(temp_time > current_pac.time){
                        //the current message can't be served because another is being served - push it to queue
                        queue_packages_arrived++;
                        service_time = temp_time;
                        total_wait_time+= service_time-current_pac.time;
                    }
                    else{
                        //we can serve this package now
                        double cur_service_time = message_time(generator);
                        service_time = current_pac.time + cur_service_time;
                        total_service_time+= cur_service_time;
                    }
                }
            }
            else{
                if(queue_packages_arrived >= waiting_size){
                    //queue is full
                    thrown_messages++;
                }
                else{
                    //message didn't finish being served but we can insert
                    queue_packages_arrived++;
                    success_messages++;
                    total_wait_time+= service_time-current_pac.time;
                }
            }
        }
        //now we serve all the messages that are still in queue but the time ran out
        double temp_time = service_time;
        while(queue_packages_arrived > 0){
            double cur_service_time = message_time(generator);
            temp_time+= cur_service_time;
            total_service_time+= cur_service_time;
            queue_packages_arrived--;
            total_wait_time+= cur_service_time*queue_packages_arrived;
        }
        service_time = temp_time;
    }
    
};

using namespace std;

int main(int argc, char *argv[]) {
    long int T = atol(argv[1]);     //time
    long int M = atol(argv[2]);     //number of out ports
    vector<double> probability;     //probalily[i] = chances to go from enter to out port i
    for (int i=0; i<M; i++) {
        probability.push_back(atof(argv[3+i]));
    }
    double enter = atof(argv[M+3]);     //enter - frames per time
    vector<long int> turn_size;     //size[i] - size of out port i
    for (int i=0; i<M; i++) {
        turn_size.push_back(atol(argv[M+4+i]));
    }
    vector<double> left;    //left[i] - frames per time that got service in out port i
    for (int i=0; i<M; i++) {
        left.push_back(atof(argv[(2*M)+4+i]));
    }
    vector<exit_port> exit_ports;
    for (int i=0; i<M; i++) {
        exit_ports.push_back(exit_port(turn_size[i], left[i]));
    }

    mt19937 engine(time(0));
    exponential_distribution<double> distribution(enter);
    double time=0.0;
    while (time < T)    {
        time = time + distribution(engine);
        package pack(time);
        double chances = 0;
        int j = 0;
        for (j=0; j<M; j++) {
            bernoulli_distribution bern(probability[j] / (1-chances));
            if (bern(engine)) break;
            chances = chances + probability[j];
        }
        exit_ports[j].waiting.push(pack);
    }

    long int total_thrown = 0;
    long int total_success = 0;
    double total_wait_time = 0;
    double total_service_time = 0;
    double last_service_time = 0;
    for (int i=0; i<M; i++) {
        exit_ports[i].simulation();
        total_thrown += exit_ports[i].thrown_messages;
        total_success += exit_ports[i].success_messages;
        total_wait_time += exit_ports[i].total_wait_time;
        total_service_time += exit_ports[i].total_service_time;
        if (last_service_time < exit_ports[i].service_time) {
            last_service_time = exit_ports[i].service_time;
        }
    }

    cout << total_success << " ";
    cout << total_thrown << " ";
    cout << last_service_time << " ";
    cout << total_wait_time / total_success << " ";
    cout << total_service_time / total_success << " ";

    return 0;
}
