#include <iostream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "message.h"
#include "uart.h"

using namespace std;
using namespace eLinux;
using namespace BBB;

uint8_t preamble[4] = {0xAA, 0xBB, 0xCC, 0xDD};
char data[30];

char **generateArgs(const char *name,
                    const char *url,
                    const char *data, 
                    const char *method);
void deleteArgs(char **args);
uint16_t request(char **args);


int main(int argc, const char** argv) {
    UART bus(UART::UART1, B9600);
    MessageBox<UART> msg(bus);
    Message_t packet;


    while (1) {
        if (msg.isAvailable()) {
            msg.pop(packet);

            printf("[From] %d\n", packet.address);

            printf("[Size] %d\n", packet.payloadSize);
        
            printf("[Payload] ");
            for (int i = 0; i < packet.payloadSize; i++) {
                printf("%02X ", packet.payload[i]);
            }
            printf("\n");
            
            sprintf(data, "cardid=%03d%03d%03d%03d", packet.payload[0],
                                                      packet.payload[1],
                                                      packet.payload[2],
                                                      packet.payload[3]);

            // Add new card id to database
            char **args = generateArgs("curl",
                                "192.168.0.99:5000/api/uid",
                                data, 
                                "POST");

            uint16_t pwd = request(args);
            cout << "Response: " << pwd << endl;
            printf("----------------------\n");

            msg.send(preamble, 1, 0, &pwd, sizeof(pwd));


            usleep(50000); /**< very important */
        }
    }
}


char **generateArgs(const char *name__,
                    const char *url__,
                    const char *data__, 
                    const char *method__) {

    const char *flag_d__ = "-d";
    const char *flag_x__ = "-X";
    const char *flag_s__ = "-s";

    #define template(var) \
        char *var = new char[strlen(var##__)+1]; \
        strcpy(var, var##__); \
    
    template(name);
    template(url);
    template(data);
    template(method);
    template(flag_d);
    template(flag_x);
    template(flag_s);
     
    char **args = new char* [8];

    args[0] = name;
    args[1] = url;
    args[2] = flag_d;
    args[3] = data;
    args[4] = flag_x;
    args[5] = method;
    args[6] = flag_s;
    args[7] = NULL;

    return args;
}


void deleteArgs(char **args) {
    uint8_t index = 0;

    while (args[index] != NULL) {
        delete[] args[index];
        index++;
    } 
    delete[] args;
}


uint16_t request(char **args) {
    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe:");
    }

    pid_t pid;
    pid = fork();

    if (pid == -1) {
        perror("fork: ");
        exit(EXIT_SUCCESS);
    }

    // in child process
    else if (!pid) {
        close(fd[0]);
        dup2(fd[1], 1);

        if (execve("/usr/bin/curl", args, NULL) == -1) {
            perror("execve: ");
            deleteArgs(args);
            exit(EXIT_SUCCESS);
        }
        deleteArgs(args);
    }
    
    // in parent process
    close(fd[1]);
    dup2(fd[0], 0);

    uint16_t response;

    cin >> response;

    return response;
}