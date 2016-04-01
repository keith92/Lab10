#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <iostream>
#include "LUrlParser.h"
#include "LUrlParser.cpp"
#include <sstream>
#include <cmath>
#include <math.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define URL_SIZE      255

using LUrlParser::clParseURL;

int  main(int argc, char* argv[])
{
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strURL[URL_SIZE];
    int sockCount = 0;
    int c, err=0;
    bool verbose=false;
    extern char *optarg;
    int nHostPort;
    clParseURL URL;
    std::string hostname;
    std::string path;
    
    if(argc < 3)
    {
        printf("\nUsage: webtest url count\n");
        return 0;
    }
    else
    {
        strcpy(strURL,argv[1]);
        URL = clParseURL::ParseURL(strURL);
        
        if(argc == 3)
            sockCount=atoi(argv[2]);
        else if(argc == 4) {
            std::string str = argv[2];
            if(str != "-d") {
                perror("Invalid operator");
                return -1;
            }
            else {
                sockCount=atoi(argv[3]);
                verbose = true;
            }
        }
    }
    
    if(URL.IsValid()) {
        hostname = URL.m_Host;
        nHostPort = atoi(URL.m_Port.c_str());
        path = URL.m_Path;
    }
    else
        perror("Invalid URL");

#define NSOCKETS sockCount
    
    struct timeval oldtime[NSOCKETS];
    struct timeval newtime[NSOCKETS];
    int hSockets[NSOCKETS];                 /* handle to socket */
    
    /* make a socket for each request */
    for(int i = 0; i < NSOCKETS; i++) {
        hSockets[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        
        if(hSockets[i] == SOCKET_ERROR)
        {
            printf("\nCould not make a socket\n");
            return 0;
        }
    }
    
    /* get IP address from name */
    pHostInfo=gethostbyname(hostname.c_str());
    if (pHostInfo == NULL) {
        std::cout << "\nInvalid Host Name" << std::endl;
        return 0;
    }
    /* copy address into long */
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);
    
    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;
    
    int epollFD = epoll_create(1);
    // Send the requests and set up the epoll data
    for(int i = 0; i < NSOCKETS; i++) {
        /* connect to host */
        if(connect(hSockets[i],(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR)
        {
            printf("\nCould not connect to host\n");
            return 0;
        }
        
        // Create request
        std::ostringstream oss;
        oss << "GET /" << path << " HTTP/1.1\r\n\r\n";
        
        write(hSockets[i],oss.str().c_str(),strlen(oss.str().c_str()));
        struct epoll_event event;
        event.data.fd = hSockets[i];
        event.events = EPOLLIN;
        int ret = epoll_ctl(epollFD,EPOLL_CTL_ADD,hSockets[i],&event);
        if(ret)
            perror ("epoll_ctl");
        gettimeofday(&oldtime[i], NULL);	//initialize timer
    }
    
    double avgTime = 0;
    double times[NSOCKETS];
    
    for(int i = 0; i < NSOCKETS; i++) {
        struct epoll_event event;
        int rval = epoll_wait(epollFD,&event,1,-1);
        if(rval < 0)
            perror("epoll_wait");
        read(event.data.fd,pBuffer,BUFFER_SIZE);
        
        gettimeofday(&newtime[i], NULL);
        double usec = (newtime[i].tv_sec - oldtime[i].tv_sec)*(double)1000000+(newtime[i].tv_usec-oldtime[i].tv_usec);
        
        times[i] = usec/1000000;
        
        if(verbose) {
            std::cout << "Request " << i+1 << ": ";
            std::cout << "Time " << times[i] <<std::endl;
        }
        
        avgTime += times[i];
        
        /* close socket */
        if(close(hSockets[i]) == SOCKET_ERROR)
        {
            printf("\nCould not close socket\n");
            return 0;
        }
    }
    
    double n = 0;
    
    for(int i = 0; i < NSOCKETS; i++) {
        n += pow(times[i]-avgTime,2);
    }
    
    double stddev = sqrt(n);
    
    std::cout << "Average time for " << NSOCKETS << " requests: " << avgTime/NSOCKETS << std::endl;
    std::cout << "Standard Deviation: " << stddev << std::endl;
    
    return 0;
}
