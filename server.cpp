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
#define BUFFSIZE 4096
#define PORT 9877

int checkSum(char pkt[], int pktLength) {
    int sum = 0;
    for(int i = 6; i < pktLength; i++) {
        sum += (int)pkt[i];
    }
    return sum;
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
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
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
        perror("bind failure");
        exit(EXIT_FAILURE);
    }

    cout << "Server starting..." << endl;
    while (true) {
        cout << "Server waiting..." << endl;
        // receive
        bzero(&receiveBuffer, BUFFSIZE);
        int len = sizeof(clientaddr);
        int receiveNum = recvfrom(sockfd, (char *)receiveBuffer, MAXLINE,
            MSG_WAITALL, ( struct sockaddr *) &clientaddr, (socklen_t *)&len);

        if (receiveNum < 0) {
            perror("error receiving from client");
            exit(EXIT_FAILURE);
        }

        // print obtained data
        receiveBuffer[receiveNum] = '\0';
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

        int segmentCount = 0;
        int byteCount = 0;

        for (int i = 0; i < (fileLength + (DATA_SIZE - 1)) / DATA_SIZE; i++) {
            char pkt[PACKET_SIZE];
            bool lastPacket = false;

            int datasize = DATA_SIZE;

            // segment data into pkt
            if (len - byteCount < PACKET_SIZE - 4) {
                datasize = len - byteCount;
                lastPacket = true;
            }

            int k = byteCount;
            for (int i = 4; i < PACKET_SIZE; i++) {
                pkt[i] = pFileContent[k];
                k++;
            }
            if (lastPacket) {
                pkt[k] = '\0';
            }

            // add sequence to header
            pkt[6] = i / 1000 % 10 + '0';
            pkt[7] = i / 100 % 10 + '0';
            pkt[8] = i / 10 % 10 + '0';
            pkt[9] = i % 10 + '0';
            
            // add checksum to header
            int chkSum = checkSum(pkt, PACKET_SIZE);
            pkt[0] = chkSum / 100000 % 10 + '0';
            pkt[1] = chkSum / 10000 % 10 + '0';
            pkt[2] = chkSum / 1000 % 10 + '0';
            pkt[3] = chkSum / 100 % 10 + '0';
            pkt[4] = chkSum / 10 % 10 + '0';
            pkt[5] = chkSum % 10 + '0';
            
            // clear sendbuffer and send
            bzero(&sendBuffer, BUFFSIZE);
            int send;
            string output = "Sending pkt ";
            output = output + to_string(segmentCount);
            output = output + " . Preview: ";
            output = output + preview(pkt);
            cout << output << endl;
            send = sendto(sockfd, (char *)sendBuffer, PACKET_SIZE, 0, (struct sockaddr *)&clientaddr, len);

            if (send < 0) {
                perror("error: sending pkt");
                exit(EXIT_FAILURE);
            }

            byteCount += datasize;
            segmentCount++;
        }
    }
    
    return 0;
}
