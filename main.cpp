#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>

#define LENGTH 512

void doprocessing (int sock);
std::vector<std::string> parseString(std::string s);
int write_to_client(int fd, char* array, int SIZE, int sz_emit);

int main( int argc, char *argv[] ) {
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, pid;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    /* Now start listening for the clients, here
       * process will go in sleep mode and will wait
       * for the incoming connection
    */

    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);

        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        /* Create child process */
        pid = fork();

        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0) {
            /* This is the client process */
            close(sockfd);
            doprocessing(newsockfd);
            exit(0);
        }
        else {
            close(newsockfd);
        }

    } /* end of while */
}

void doprocessing (int sock) {
    int n;
    char buffer[256];
    bzero(buffer,256);
    n = recv(sock,buffer,255,0);

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }
    std::string command(buffer);
    std::vector<std::string> vstrings = parseString(command);
    if(vstrings.at(0) == "GET"){
        if(vstrings.size()<3 || vstrings.size()>3){
            //error msg
        }else {
            n = write(sock,"HTTP/1.0 200 OK \n \n",17);
            std::string file_path_string = "/home/ehab/ClionProjects/Server" + vstrings.at(1);
            char sdbuf[LENGTH];
            printf("[Server] Sending %s to the Client... ", file_path_string.c_str());
            FILE *fs = fopen(file_path_string.c_str(), "r");
            if(fs == NULL)
            {
                printf("ERROR: File %s not found.\n", file_path_string.c_str());
                exit(1);
            }

            bzero(sdbuf, LENGTH);
            int fs_block_sz;
            while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0)
            {
                n=send(sock, sdbuf, fs_block_sz,0);
                if(n<0) {
                    fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", file_path_string.c_str(), errno);
                    break;
                }
                bzero(sdbuf, LENGTH);
            }
            printf("Ok File %s from Server was Sent!\n", file_path_string.c_str());
            if (n < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
        }
    }else if(vstrings.at(0) == "POST"){

    }

    printf("Here is the message: %s\n",buffer);

}

std::vector<std::string> parseString(std::string s){
    std::stringstream ss(s);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vstrings(begin, end);
    std::copy(vstrings.begin(), vstrings.end(), std::ostream_iterator<std::string>(ss));
    return vstrings;
}

//in @param
//@param fd the socket file descriptor
//@param array an array of data source to write to send to the connected client
//@param SIZE the size of data source to send to the client
//@param sz_emit the size of data to send in one loop step
//out @param
//total length of data emited to the client

int write_to_client(int fd, char* array, int SIZE, int sz_emit)
{
    //#######################
    //    server code
    //#######################
    int i=0, sz=0;
    for(i = 0; i < SIZE; i += sz_emit )
    {
        while(sz_emit-sz)
        {
            sz+=write(fd, array+i+sz, sz_emit-sz);
        }
        sz = 0;
    }
    return i;
}