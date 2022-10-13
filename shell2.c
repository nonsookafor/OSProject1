//Teammate: David Oluyomi-Lords

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[256];
char delimiters[] = " \t\r\n";
extern char **environ;
int main(void);
pid_t pid1, pid2; //pid variables to store pid i'll kill later

//Task 4
void handler(int signum) {
	kill(-9, pid1); //kill long runing processes from before SIGINt was called
	printf("\n");
}

//Task 5
void handler2(int signum) {
	kill(-9, pid2); //kill process after 10 seconds if the shell isnt completed
}

int main() {
    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
		char *cmd;
		int background = 0;
		int lastIndex;
  
    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS], *envVariables[MAX_COMMAND_LINE_ARGS];
    int i, j, k;   	
		pid1 = getpid();
		signal(SIGINT, handler);
    while (true) {    
        do{ 
            // Print the shell prompt.
            //Step 0 Modify the prompt to print the current working directory
            if (getcwd(prompt, sizeof(prompt)) == NULL)
                perror("getcwd() error");
            else
                printf("%s>", prompt);
                fflush(stdout);

            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")  
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
 
        }while(command_line[0] == 0x0A);  // while just ENTER pressed
      
        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }

        //Step 1 Tokenize the command line input (split it on whitespace)
        arguments[0] = strtok(command_line, delimiters);
        i = 0;
				lastIndex = 0; //getting last index so we can later check if it is a background process
        while (arguments[i] != NULL) {
            i ++;
						lastIndex ++;
            arguments[i] = strtok(NULL, delimiters);
        }

				// handling background process
				if (((size_t)strcmp((char*)arguments[lastIndex-1], (char*)"&") == (size_t)0)) {
					arguments[lastIndex-1] = NULL;
					pid_t  pid1;
					pid1 = fork();
					if (pid1 == 0) {
						cmd = arguments[0];
						background = 1;
						if (execvp(cmd, arguments) == -1) {
							printf("execvp() failed: No such file or directory\nAn error occurred.\n");
						}
						else {
							execvp(cmd, arguments);
						}
						// exit(1);
					}
					else if (pid1 < 0) {
						printf("Fork failed");
						// exit(1);
					}
				}

				else {
					//Step 2 Implement Built-In Commands
					if(strcmp(arguments[0], "cd") == 0) {
							chdir(arguments[1]);
					}
					else if(strcmp(arguments[0], "pwd") == 0) {
							printf("%s\n", prompt);
					}
					else if(strcmp(arguments[0], "echo") == 0) {
							i = 1;
							while (arguments[i] != NULL) {
									if (*arguments[i] == '$') {
											printf("%s ", getenv((arguments[i] + 1)));
									}
									else {
											printf("%s ", arguments[i]);
									}
									i ++;
							}
							printf("\n");
					}
					else if(strcmp(arguments[0], "exit") == 0) {
							exit(0);
					}
					else if(strcmp(arguments[0], "env") == 0) {
							if (arguments[1] != NULL) {
									printf("%s\n", getenv(arguments[1]));
							}
							else {
									while(environ[j]) {
											printf("%s\n", environ[j]); 
											j ++;
									}
							}
					}
					else if(strcmp(arguments[0], "setenv") == 0) {
							i = 1;
							while (arguments[i] != NULL) {
									envVariables[0] = strtok(arguments[i], "=");
									k = 0;
									while (envVariables[k] != NULL) {
											k ++;
											envVariables[k] = strtok(NULL, "=");
									}
									setenv(envVariables[0], envVariables[1], 1);
									i ++;
							}
					}
	
					// 3. Create a child process which will execute the command line input
					else {
						pid_t  pid;
						pid = fork();
						if (pid == 0) {
							//Task 5
							//sigalrm to set a timer of ten minutes for each 
							pid2 = getpid();
							signal(SIGALRM, handler2);
							alarm(10);
							cmd = arguments[0];
							if (execvp(cmd, arguments) == -1) {
								printf("execvp() failed: No such file or directory\nAn error occurred.\n");
							}
							else {
								execvp(cmd, arguments);
							}
						}
						//Step 4 The parent process should wait for the child to complete unless its a background process
						else if (pid > 0) {
							if (!background) {
								wait(NULL);
							}
							// printf("Child Complete\n");
						}
						else {
							printf("Fork failed");
						}
					}
				}
    }
    // This should never be reached.
    return -1;
}

