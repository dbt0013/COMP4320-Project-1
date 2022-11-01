#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void gremlin(char packet[], int packetLength, double probability) {
    double randValue = rand() / RAND_MAX;
    if (randValue > probability) {
        return;
    }
    
    if (randValue >= 0 && randValue < 0.5) {
        packet[(int)(randValue * packetLength)] = '0';
    }
    else if (randValue >= 0.5 && randValue < 0.8) {
        for (int i = 0; i < 2; i++) {
            packet[(int)(randValue * packetLength)] = '0';
        }
    } 
    else {
        for (int i = 0; i < 3; i++){
            packet[(int)(randValue * packetLength)] = '0';
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        error("error: no IP");
    }

    return 0;
}
