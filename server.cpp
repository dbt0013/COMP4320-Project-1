#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>


#DEFINE PACKET_SIZE = 512

void checkSum(char[] pkt, int length) {
    int sum = 0;

    for (int i = 7; i < pkt.length; i++) {
        sum += (int)c[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {

    return 0;
}