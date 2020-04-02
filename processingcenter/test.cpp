#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <string.h>

using namespace std;


char **generateArgs(const char *name__,
                    const char *url__,
                    const char *data__, 
                    const char *method__) {

    #define template(var) \
        char *var = new char[strlen(var##__)+1]; \
        strcpy(var, var##__); \
    
    template(name);
    template(url);
    template(data);
    template(method);

    char **args = new char*[8];
    args[0] = name;
    args[1] = url;
    args[2] = "-d";
    args[3] = data;
    args[4] = "-X";
    args[5] = method;
    args[6] = "-s";
    args[7] = NULL;

    return args;
}


int main(int argc, const char **argv) {

    char **args = generateArgs("curl",
                                "http://localhost:5000/api/uid",
                                argv[1], 
                                "POST");
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
            exit(EXIT_SUCCESS);
        }
    }
    
    close(fd[1]);
    dup2(fd[0], 0);

    printf("My child: %d\n", pid);
    char message[30];

    while (fgets(message, sizeof(message), stdin)) {
        printf("%s\n", message);
    }
    
}