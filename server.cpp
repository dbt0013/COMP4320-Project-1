#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>

using namespace std;

#DEFINE PACKET_SIZE = 512
#DEFINE HEADER_SIZE = 8
#DEFINE DATA_SIZE = 504
#DEFINE BUFFSIZE = 4096
#DEFINE PORT = 8977

void checkSum(char[] pkt, int length) {
    int sum = 0;

    for (int i = 8; i < pkt.length; i++) {
        sum += (int)c[i];
    }
    return sum;
}

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int sockfd;

    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    char sendBuffer[BUFFSIZE]
    char receiveBuffer[BUFFSIZE]
    // create socket file descriptor
    // through error if it fails
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("Socket creation failed");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&clientaddr, 0, sizeof(clientaddr));


    // fill server information
    servaddr.sin_family = AF_INET; //IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind socket with server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr,
        sizeof(servaddr)) < 0) {
        perror("bind failure")
        exit(EXIT_FAILURE);
    }

    cout << "Server starting..." << endl;
    while (true) {
        // receive
        bzero(&receiveBuffer, BUFFSIZE);
        int len = sizeof(clientaddr)
        int n = recvfrom(sockfd, (char *)receiveBuffer, MAXLINE,
            MSG_WAITALL, ( struct sockaddr *) &clientaddr, &len);

        if (n < 0) {
            perror("error receiving from client");
            exit(EXIT_FAILURE);
        }

        // print obtained data
        receiveBuffer[n] = '\0';
        printf("Client : %s\n", receiveBuffer);

        // get file request
        char filename[128];
        char *pFileContent;
        sscanf(receiveBuffer, "GET %s HTTP/1.0", filename);
        FILE *pf = fopen(filename, "r");

        if (pf == NULL) {
            perror("error: file not found");
            exit(EXIT_FAILURE)
        }

        fseek(pf, 0, SEEK_END);
        fileLength = ftell(pf);

        pFileContent = (char *)malloc(sizeof(char) * fileLength);

        if (pfilecont == NULL) {
            perror("error: memory allocation");
            exit(EXIT_FAILURE);
        }

        rewind(pf);
        fread(pFileContent, sizeof(char), fileLength, pf);
        fclose(pf);
    }

    
    return 0;
}