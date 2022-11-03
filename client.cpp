#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/stat.h>

using namespace std;

#define SERV_PORT 9877 //default port
#define MAXLINE 4096 
#define BUFFSIZE 8192
#define PACKET_SIZE 512
#define HEADER_SIZE 8
#define DATA_SIZE 504


int checksum(char pkt[], int pktLength) {
    int sum = 0;
    for(int i = 4; i < pktLength; i++) {
        sum += (int)pkt[i];
    }
    return sum;
}

void gremlin(char pkt[], int pktLength, float probability) {
    double randValue = rand() / RAND_MAX;
    if (randValue > probability) {
        return;
    }
    
    if (randValue >= 0 && randValue < 0.5) {
        pkt[(int)(randValue * pktLength)] = '0';
    }
    else if (randValue >= 0.5 && randValue < 0.8) {
        for (int i = 0; i < 2; i++) {
            pkt[(int)(randValue * pktLength)] = '0';
        }
    } 
    else {
        for (int i = 0; i < 3; i++){
            pkt[(int)(randValue * pktLength)] = '0';
        }
    }
}


bool checkPkt(char pkt[], int pktLength) {
    int count = 100000;
    int sum;
    for (int i = 0; i < 6; i++) {
        sum += (pkt[i] - '0') * count;
        count /= 10;
    }
    return sum == checksum(pkt, pktLength);
}


void reassemblePkt(char pkt[], char *content, int pktLength) {
    int count = 1000;
    int seq;
    for (int i = 6; i < 10; i++) {
        seq += (pkt[i] - '0') * count;
        count /= 10;
    }

    for (int i = 0; i < pktLength - HEADER_SIZE; i++) {
        content[seq * DATA_SIZE + i] = pkt[i + HEADER_SIZE];
    }
}


int main(int argc, char *argv[]) {
    while (true) {
        srand(time(NULL));
        struct sockaddr_in servaddr; // Server address
        struct sockaddr_in cliaddr; // Client address
        char sendBuffer[BUFFSIZE];
        char receiveBuffer[BUFFSIZE];
        float errorProbability;
        int fileLength;

        // Check input IP
        if (argc != 2) {
            perror("error: no IP");
            exit(EXIT_FAILURE);
        }

        char *phostname = argv[1];
        // Get valid IP from args
        struct hostent *pservname = gethostbyname(phostname);
        if (pservname == NULL) {
            perror("error: no server name");
            exit(EXIT_FAILURE);
        }

        // Create socket
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == 0) {
            perror("error: no socket");
            exit(EXIT_FAILURE);
        }

        // Initialize client
        bzero(&servaddr, sizeof(struct sockaddr_in));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(SERV_PORT);
        servaddr.sin_addr.s_addr = inet_addr(phostname);
        int serverLength = sizeof(servaddr);

        // Start output
        cout << "Client start:" << endl;

        // Get packet error probability
        while (true) {
            cout << "Enter packet error probability [0, 1]: ";
            cin >> errorProbability;
            if (errorProbability >= 0 && errorProbability <= 1) {
                break;
            }
            cout << "Invalid probability, please input a value between 0 and 1" << endl;
        }    

        // Send filename to server
        bzero(&sendBuffer, BUFFSIZE);
        char filename[128];
        cout << "Enter the requested filename: ";
        cin >> filename;
        
        // Construct request to send
        string request = "GET ";
        request += filename;
        request += " HTTP/1.0";
        cout << "client send: " << request << endl;
        strcpy(sendBuffer, request.c_str());
        int sendNum = sendto(sockfd, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&servaddr, serverLength);
        if (sendNum < 0) {
            perror("error: sendto");
            exit(EXIT_FAILURE);
        }
        
        // Receive server response
        int receivedNum = recvfrom(sockfd, receiveBuffer, BUFFSIZE, 0, (struct sockaddr *)&servaddr, (socklen_t *)&serverLength);
        if (receivedNum < 0) {
            perror("error: recvfrom");
            exit(EXIT_FAILURE);
        }
        cout << "Server response: \n" << receiveBuffer;
        
        // Get length of file
        receivedNum = recvfrom(sockfd, receiveBuffer, BUFFSIZE, 0, (struct sockaddr *)&servaddr, (socklen_t *)&serverLength);
        if (receivedNum < 0) {
            perror("error: recvfrom");
            exit(EXIT_FAILURE);
        }
        sscanf(receiveBuffer, "%d", &fileLength);
        if (fileLength == -1) {
            cout << "client end" << endl;
            break;
        }

        // Open file stream
        char *pFileContent = (char *)malloc(sizeof(char) * fileLength);
        FILE *pFile = fopen(filename, "w");
        if (pFile == NULL) {
            perror("error: open file");
            exit(EXIT_FAILURE);
        }

        // Receive packet
        cout << "Receiving packet..." << endl;
        while(true) {
            receivedNum = recvfrom(sockfd, receiveBuffer, BUFFSIZE, 0, (struct sockaddr *)&servaddr, (socklen_t *)&serverLength);
            if (receivedNum < 0) {
                perror("error: revfrom");
                exit(EXIT_FAILURE);
            }

            if (receivedNum == 0) {
                cout << "all packets received" << endl;
                break;
            }

            // Temporary packet
            int pktLength = receivedNum;
            char pkt[pktLength];
            memset(&pkt, 0, sizeof(pkt));
            for (int i = 0; i < pktLength; i++) {
                pkt[i] = receiveBuffer[i];
            }

            // Gremlin
            gremlin(pkt, sizeof(pkt), errorProbability);
            int count = 1000;
            int seq;
            for (int i = 6; i < 10; i++) {
                seq += (pkt[i] - '0') * count;
                count /= 10;
            }
            cout << "Received packet [" << seq << "] size: " << pktLength << " bytes from server | ";

            // Print packet and check result
            if (checkPkt(pkt, pktLength)) {
                cout << "pass" << endl;
            }
            else {
                cout << "error" << endl;
            }

            reassemblePkt(pkt, pFileContent, pktLength);
        }

        // Write to file
        fwrite(pFileContent, sizeof(char), fileLength, pFile);
        fclose(pFile);
        free(pFileContent);
        cout << "file: " << filename << " has been obtained" << endl;
        char con;
        while(true) {
            cout << "continue(y/n): ";
            cin >> con;
            if(con == 'y' || con == 'n') {
                break;
            }
            cout << "wrong input" << endl;
        }
        if (con == 'n') {
            cout << "client end" << endl;
            break;
        }
    }
    return 0;
}
