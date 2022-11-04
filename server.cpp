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
#include <vector>

using namespace std;

#define PACKET_SIZE 512
#define HEADER_SIZE 10
#define DATA_SIZE 502
#define MAXLINE 4096
#define BUFFSIZE 8192
#define PORT 9877

int checkSum(char pkt[], int pktLength) {
    int checksum = 0;

    for (int i = 7; i < len; i++) {
        checksum += (int)pkt[i];
    }
    return checksum;
}

string preview(char* p) {
    string prev = "";
    for (int i = 4; i < 54; i++) {
        if (p[i] == '\0') {
            break;
        }
        prev = prev + p[i];
    }

    return prev;
}

string intToString(int input) {
    stringstream ss;
    ss << input;
    return ss.str();
}

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int sockfd;

    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    char sendBuffer[BUFFSIZE];
    char receiveBuffer[BUFFSIZE];
    // create socket file descriptor
    // through error if it fails
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&clientaddr, 0, sizeof(clientaddr));
    
    bzero(&servaddr, sizeof(struct sockaddr_in));

    // fill server information
    servaddr.sin_family = AF_INET; //IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    int serverLength = sizeof(servaddr);
    // Bind socket with server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr,
        sizeof(servaddr)) < 0) {
        perror("bind failure");
        exit(EXIT_FAILURE);
    }

    int clientLength = sizeof(clientaddr);
    cout << "Server starting..." << endl;
    while (true) {
        cout << "Server waiting..." << endl;
        // receive
        bzero(&receiveBuffer, BUFFSIZE);
        int n = recvfrom(sockfd, (char *)receiveBuffer, MAXLINE,
            MSG_WAITALL, ( struct sockaddr *) &clientaddr, (socklen_t *)&clientLength);

        if (n < 0) {
            perror("error receiving from client");
            exit(EXIT_FAILURE);
        }

        // print obtained data
        cout << "Request from clinet" << endl;
        receiveBuffer[n] = '\0';
        printf("Client : %s\n", receiveBuffer);

        // get file request
        char filename[128];
        char *pFileContent;
        sscanf(receiveBuffer, "GET %s HTTP/1.0", filename);
        FILE *pf = fopen(filename, "r");

        if (pf == NULL) {
            perror("error: file not found");
            exit(EXIT_FAILURE);
        }

        fseek(pf, 0, SEEK_END);
        int fileLength = ftell(pf);

        pFileContent = (char *)malloc(sizeof(char) * fileLength);

        if (pFileContent == NULL) {
            perror("error: memory allocation");
            exit(EXIT_FAILURE);
        }

        rewind(pf);
        fread(pFileContent, sizeof(char), fileLength, pf);
        fclose(pf);

        //send file length
        string filelength = intToStr(filelen);
        strcpy(sendbuf, filelength.c_str());
        sendnum = sendto(sockfd, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&cliaddr, clilen);
        if(sendnum < 0) {
            perror("error: sendto");
            exit(1);
        }
        
        //packet[0-5]: checksum
        //packet[6-9]: sequence
        //packet[10-511]: data
        //if it is the last packet, the length will < 512
        cout << "sending..." << endl;
        for (int i = 0; i < (fileLength + (DATA_SIZE - 1)) / DATA_SIZE; i++) {
            bzero(&sendBuffer, BUFFSIZE);
            bool lastPacket = false;
            if (i > 9999) {
                perror("error: file is too large");
                exit(EXIT_FAILURE);
            }
            //determine whether it is the last package
            int pktlen = PACKET_SIZE;
            if (i == fileLength / DATA_SIZE) {
                pktlen = fileLength % DATA_SIZE + HEADER_SIZE;
                lastPacket = true;
            }
            char pkt[pktlen];
            memset(&pkt, 0, sizeof(pkt));
            //put sequence header
            pkt[6] = i / 1000 % 10 + '0';
            pkt[7] = i / 100 % 10 + '0';
            pkt[8] = i / 10 % 10 + '0';
            pkt[9] = i % 10 + '0';
            //put data
            for(int j = HEADER_SIZE; j < pktlen; j++) {
                pkt[j] = pFileContent[(i * DATA_SIZE) + (j - HEADER_SIZE)];
            }
            int csum = checkSum(pkt, pktlen);
            //put checksum header
            pkt[0] = csum / 100000 % 10 + '0';
            pkt[1] = csum / 10000 % 10 + '0';
            pkt[2] = csum / 1000 % 10 + '0';
            pkt[3] = csum / 100 % 10 + '0';
            pkt[4] = csum / 10 % 10 + '0';
            pkt[5] = csum % 10 + '0';
            //put packet in buff
            for(int j = 0; j < pktlen; j++) {
                sendBuffer[j] = pkt[j];
            }
            //send
            cout << "Sent packet [" << i << "] size: " << sizeof(pkt) << " bytes to client" << endl;
            n = sendto(sockfd, (char *)sendBuffer, pktlen, 0, (struct sockaddr *)&cliaddr, clientLength);
            if (n < 0) {
                perror("error: Sending packet");
                exit(EXIT_FAILURE);
            }
            //send NULL after sending all packet
            if(lastPacket) {
                cout << "Full file sent to client" << endl;
                bzero(&sendBuffer, BUFFSIZE);
                n = sendto(sockfd, (char *)sendBuffer, 0, 0, (struct sockaddr *)&cliaddr, clientLength);
                if (n < 0) {
                    perror("error: Sending last packet");
                    exit(1);
                }
            }
        }
        cout << endl;
        free(pFileContent);
    }
    
    return 0;
}
