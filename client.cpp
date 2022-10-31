#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

void error(char *msg)
{
    perror(msg)
    exit(EXIT_FAILURE)
}

int main()
{
    return 0;
}
