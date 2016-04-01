#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <sstream>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255
#define PATH_TO_FILE_SIZE   255

using namespace std;

int  main(int argc, char* argv[])
{
    int hSocket;                 /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address struct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int nHostPort;
    char pathToFile[PATH_TO_FILE_SIZE];
    int c, downloadcnt=1, err=0;
    bool debug=false;
    extern char *optarg;
    
    if(argc < 4)
    {
        printf("\nUsage: download host-name host-port file-path\n");
        return 0;
    }
    else
    {
        while ((c = getopt(argc, argv, "c:d")) != -1) {
            switch (c) {
                case 'c':
                    downloadcnt = atoi(optarg);
                    break;
                case 'd':
                    debug = true;
                    break;
                case '?':
                    err = 1;
                    break;
            }
        }
        strcpy(strHostName,argv[optind]);
        nHostPort=atoi(argv[optind + 1]);
        strcpy(pathToFile,argv[optind + 2]);
    }
    
    for(int i = 0; i < downloadcnt; i++) {
    
        if(debug)
            printf("\nMaking a socket");
        /* make a socket */
        hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    
        if(hSocket == SOCKET_ERROR)
        {
            perror("\nCould not make a socket");
            return 0;
        }
    
        /* get IP address from name */
        pHostInfo=gethostbyname(strHostName);
        if (pHostInfo == NULL) {
            cout << "\nInvalid Host Name" << endl;
            return 0;
        }
        /* copy address into long */
        memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);
    
        /* fill address struct */
        Address.sin_addr.s_addr=nHostAddress;
        Address.sin_port=htons(nHostPort);
        Address.sin_family=AF_INET;
    
        if(debug)
            printf("\nConnecting to %s (%X) on port %d\n",strHostName,nHostAddress,nHostPort);
    
        /* connect to host */
        if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR)
        {
            perror("\nCould not connect to host");
            return 0;
        }
    
    #define MAXMSG 1024
    
        char *message = (char *)malloc(MAXMSG);
        sprintf(message, "GET %s HTTP/1.1\r\nHost:%s:%d\r\n\r\n",pathToFile, strHostName, nHostPort);
    
        if(debug)
            printf("Message:\n%s\n",message);
    
        write(hSocket,message,strlen(message));
        memset (pBuffer, 0, BUFFER_SIZE);
    
        //scan buffer for \r\n\r\n
        bool found = false;
        vector<string> headers;
        ostringstream newstr;
        ostringstream body;
    
        while (found == false)
        {
            nReadAmount=read(hSocket,pBuffer,BUFFER_SIZE);
        
            char *ptr = NULL;
            ptr = &pBuffer[0];
            int pos = 0;
        
            for(int j = 0; j < BUFFER_SIZE; j++) {
                newstr << *ptr;
                if(*ptr == '\n') {
                    if(newstr.str() == "\r\n") {
                        pos = j;
                        found = true;
                        break;
                    }
                    else {
                        headers.push_back(newstr.str());
                        newstr.str("");
                        newstr.clear();
                    }
                }
                ptr = &pBuffer[j+1];
            }
        
            if(pos != 0) {
                for(int j = pos; j < BUFFER_SIZE; j++) {
                    body << *ptr;
                    ptr = &pBuffer[j+1];
                }
            }
            memset (pBuffer, 0, BUFFER_SIZE);
        }
    
        int contentlength = 0;
    
        if(debug)
            cout << "Response\n";
    
        for(int j = 0; j < headers.size(); j++) {
            size_t foundstr = headers.at(j).find("Content-Length:");
            if(foundstr != string::npos) {
                string str = headers.at(j).substr(15);
                contentlength = atoi(str.c_str());
            }
            if(debug)
                cout << headers.at(j);
        }
    
        int contentleft = contentlength - body.str().size();
    
        nReadAmount=read(hSocket,pBuffer,contentleft);
        body << pBuffer;
    
        if(downloadcnt == 1)
            cout << body.str() << endl;
        
        free(message);
    
        /* close socket */
        if(close(hSocket) == SOCKET_ERROR)
        {
            perror("\nCould not close socket\n");
            return 0;
        }
    }
    if(downloadcnt > 1)
        cout << "Page Successfully Downloaded " << downloadcnt << " times" << endl << endl;
}
