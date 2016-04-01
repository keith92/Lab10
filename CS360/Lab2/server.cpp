#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include "CS360Utils.h"
#include <vector>
#include "CS360Utils.cpp"
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <sys/signal.h>

int portno = 0;
std::string rootDir;

void writeImage(int skt, std::string rsrc) {
    //write an image file to the socket
    
    FILE *file;
    unsigned char *buffer;
    unsigned long fileLen;
    //Open file
    file = fopen(rsrc.c_str(), "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s", rsrc.c_str());
        return;
    }
    //Get file length
    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);
    //Allocate memory
    buffer=(unsigned char *)malloc(fileLen);
    if (!buffer)
    {
        fprintf(stderr, "Memory error!");
        fclose(file);
        return;
    }
    int read_size = fread(buffer,fileLen,sizeof(unsigned char),file);
    fclose(file);
    
    int write_amt = 0;
    while (write_amt < fileLen) {
        write_amt += write(skt, buffer, fileLen);
    }
}

void writeText(int skt, std::string rsrc) {
    // write a text based file to the socket
    std::string body;
    int c;
    FILE *file;
    file = fopen(rsrc.c_str(), "r");
    if (file) {
        while ((c = getc(file)) != EOF) {
            size_t len = strlen(body.c_str());
            char *str2 = (char*)malloc(len + 1 + 1 ); // one for extra char, one for trailing zero
            strcpy(str2, body.c_str());
            str2[len] = c;
            str2[len + 1] = '\0';
            body = str2;
            
            free( str2 );
        }
        fclose(file);
    }
    
    else cout << "Unable to open file";
    
    int writeBytes = write(skt, body.c_str(), body.size());
}

void send404(int skt) {
    // create a 404 file not found page and write it to the socket
    std::string msg = "HTTP/1.1 404 Not Found\r\nContent-Length: 53\r\n\r\n<html><body><h1>404 File Not Found</h1></body></html>";
    int writeBytes = write(skt, msg.c_str(), msg.size());
    std::cout << "Response: \n" << msg << std::endl;
}

void makeDirHTML(int skt, std::string dir, vector<std::string> files) {
    // make HTML page for the directory file listing
    std::ostringstream oss;
    oss << "<html><body><h1>Current Directory</h1>";
    
    for(int i = 0; i < files.size(); ++i) {
        // create the html for each link to the directory files
        std::ostringstream link;
        link << dir << files[i];
        oss << "<a href=\"" << link.str() << "\">" << dir << files[i] << "</a></br>";
    }
    
    oss << "</body></html>";
    
    // write the html to the socket
    std::string dirHTML = oss.str();
    std::ostringstream ossmsg;
    ossmsg << "HTTP/1.1 200 OK\r\nContent-Length: " << dirHTML.size() << "\r\n\r\n";
    std::string msg = ossmsg.str();
    int msgBytes = write(skt, msg.c_str(), msg.size());
    int bodyBytes = write(skt, dirHTML.c_str(), dirHTML.size());
    std::cout << "Response: \n" << msg << std::endl;
}

void serve(int skt) {
    
    //read in request headers
    CS360Utils utils;
    vector<char*> headers;
    utils.GetHeaderLines(headers, skt, false);
    
    // parse request headers
    for(int i = 0; i < headers.size(); i++) {
        std::cout << headers[i] << std::endl;
    }
    std::cout << "***************************************" << std::endl;
    
    // find requested resource in request headers
    std::string header0 = headers[0];
    std::size_t pos = header0.find("GET");
    pos = pos + 4;
    header0 = header0.substr(pos);
    pos = header0.find(" ");
    header0 = header0.substr(0, pos);
    std::string rsrc = rootDir + header0;
    
    // determine if requested resource is file or directory
    struct stat filestat;
    DIR *dirp;
    struct dirent *dp;
     
    if(stat(rsrc.c_str(), &filestat)) {
        // rsrc doesn't exist
        // send 404 page not found
        send404(skt);
        return;
    }
     
    if(S_ISREG(filestat.st_mode)) {
        // rsrc is a file
        
        // create headers and response message
        int fileSize = filestat.st_size;
        std::ostringstream oos;
        oos << "Content-Length: " << fileSize << "\r\n";
        std::string fileSizeHeader = oos.str();
        
        int exPos = header0.find(".");
        std::string rsrcExtension = header0.substr(exPos);
        std::string rsrcType;
        if(rsrcExtension == ".html")
            rsrcType = "text/html";
        else if(rsrcExtension == ".txt")
            rsrcType = "text/plain";
        else if(rsrcExtension == ".gif")
            rsrcType = "image/gif";
        else if(rsrcExtension == ".jpg")
            rsrcType = "image/jpg";
        
        oos.str("");
        oos.clear();
        oos << "Content-Type: " << rsrcType << "\r\n";
        std::string typeHeader = oos.str();
        oos.str("");
        oos.clear();
        oos << "HTTP/1.1 200 OK\r\n" << typeHeader << fileSizeHeader << "\r\n";
        std::string msg = oos.str();
        
        // write response to socket
        int writeBytes = write(skt, msg.c_str(), msg.size());
        std::cout << "Response: \n" << msg << std::endl;
        
        // write file to socket
        if(rsrcExtension == ".html" || rsrcExtension == ".txt")
            writeText(skt, rsrc);
        else if(rsrcExtension == ".gif" || rsrcExtension == ".jpg")
            writeImage(skt, rsrc);
    }
    
    if(S_ISDIR(filestat.st_mode)) {
        //rsrc is a directory
        
        // copy all file names in directory to a vector
        dirp = opendir(rsrc.c_str());
        vector<std::string> files;
        while ((dp = readdir(dirp)) != NULL)
            files.push_back(dp->d_name);
        
        // search vector for index.html
        bool foundIndex = false;
        for(int i = 0; i < files.size(); i++) {
            if(files[i] == "index.html")
                foundIndex = true;
        }
        
        // send back index.html if it exists
        if(foundIndex) {
            struct stat filestat2;
            std::string rsrc2 = rsrc + "/index.html";
            stat(rsrc2.c_str(), &filestat2);
            int fileSize = filestat2.st_size;
            std::cout << "Resource: " << rsrc2 << std::endl << "File Size: " << fileSize << std::endl;
            std::ostringstream oss;
            oss << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << fileSize << "\r\n\r\n";
            std::string msg = oss.str();
            int writeBytes = write(skt, msg.c_str(), msg.size());

            writeText(skt, rsrc2);
        }
        // send back a directory file listing
        else {
            makeDirHTML(skt, header0, files);
        }
        
        (void)closedir(dirp);
    }
}

void handler (int status)
{
    printf("received signal %d\n",status);
}

int main(int argc, char* argv[]) {
    
    // parse input parameters
    if(argc != 3) {
        std::cout << ":(" << std::endl;
        exit(0);
    }
    
    portno = atoi(argv[1]);
    rootDir = argv[2];
    
    //create a socket
    int skt = socket(AF_INET, SOCK_STREAM, 0);
    if(skt == -1) {
        std::cout << "no socket :(" << std::endl;
        exit(0);
    }
    
    // setup socket address
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(portno);
    servAddr.sin_addr.s_addr = INADDR_ANY;
    
    //bind socket to port
    int optval = 1;
    setsockopt (skt, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    int bindReturn = bind(skt, (sockaddr*) &servAddr, (socklen_t) sizeof(servAddr));
    
    if(bindReturn == -1) {
        std::cout << "can't bind :(" << std::endl;
        exit(0);
    }
    
    //tell socket to listen
    
    int listenReturn = listen(skt, 1000);
    
    //accept connection
    
    int rc1, rc2;
    
    // First set up the signal handler
    struct sigaction sigold, signew;
    
    signew.sa_handler=handler;
    sigemptyset(&signew.sa_mask);
    sigaddset(&signew.sa_mask,SIGINT);
    signew.sa_flags = SA_RESTART;
    sigaction(SIGHUP,&signew,&sigold);
    sigaction(SIGPIPE,&signew,&sigold);
    
    for(;;)
    {
        // make empty sockaddr to fill in when connection is made
        struct sockaddr_in clientAddr;
        int sock_len = sizeof(sockaddr_in);
        int conn_sock = accept(skt, (sockaddr*) &clientAddr, (socklen_t*)&sock_len);
        
        if(conn_sock == -1) {
            std::cout << "unsuccessful connection :(" << std::endl;
        }
        
        //serve resource
        serve(conn_sock);
        
        close(conn_sock);
    }
    return 0;
}
