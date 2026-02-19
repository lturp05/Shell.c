// Name(s): Christopher Sandoval, Luke Turpin
// Description:


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

struct ShellCommand{
    char *cmd; // Command
    char *args[MAX_ARGS]; // Arguments list
    char *inputFile; // This is "<" redirection file
    char *outputFile; // THis is ">" redirection file
    int redirectInput;
    int redirectOutput;
};

void displayPrompt();
char* getInput();
struct ShellCommand parseInput(char* input);
void executeCommand(struct ShellCommand command);


int main() // MAIN
{
	char* input;
	struct ShellCommand command;
		
	// repeatedly prompt the user for input
	for (;;)
	{
        // display the prompt
        displayPrompt();

	    // get the user's input
	    input = getInput();
	    
	    // parse the command line
	    command = parseInput(input);
	    
	    // execute the command
	    executeCommand(command);

        free(input);
	}

	exit(0);
}

void displayPrompt(){
    char cwd[1024]; 

    if (getcwd(cwd, sizeof(cwd)) != NULL) { // Get the current working directory
        printf("%s$ ", cwd); // Display the current working directory followed by a $ symbol
    } else {
        printf("error %d: %s\n", errno, strerror(errno)); // Print an error message if getcwd fails
    }
}

char* getInput(){
    char *buffer = malloc(MAX_INPUT_SIZE); // Allocate memory for the input buffer
    if(!buffer){ // Checks for successful memory allocation
        fprintf(stderr, "error %d: %s\n", errno, strerror(errno));
        exit(1);
    }

    if(fgets(buffer, MAX_INPUT_SIZE, stdin) == NULL){ // Read the imput from the user and check for errors
        printf("\n"); 
        free(buffer); // Free the allocated memory before exiting
        exit(0);
    }

    buffer[strcspn(buffer, "\n")] = '\0'; // Remove the newline from the end of input
    return buffer; 
}

struct ShellCommand parseInput(char *input){
    struct ShellCommand cmd; 
    memset(&cmd, 0, sizeof(cmd)); // Initialize the ShellCommand struct to zero

    char *token = strtok(input, " "); 
    int argIndex = 0; 

    while(token != NULL){ // Loop through each token in the input
        if(strcmp(token, "<") == 0){ // Check for input redirection symbol
            cmd.redirectInput = 1; 
            token = strtok(NULL, " "); // Get the next token which should be the input file
            cmd.inputFile = token; // Store the input file in the ShellCommand struct
        }
        else if (strcmp(token, ">") == 0){ // Check for output redirection symbol
            cmd.redirectOutput = 1;
            token = strtok(NULL, " ");
            cmd.outputFile = token;
        }
        else{
            if(cmd.cmd == NULL){ // If the command has not been set yet, set it to the current token
                cmd.cmd = token;
            }
            if(argIndex < MAX_ARGS - 1){ // Check if there is room for more arguments
                cmd.args[argIndex++] = token;
            }
        }

        token = strtok(NULL, " "); // Get the next token for the next iteration of the loop
    }

    cmd.args[argIndex] = NULL;
    return cmd;
}

void executeCommand(struct ShellCommand command){
    if(command.cmd == NULL){
        return;
    }

    if(strcmp(command.cmd, "cd") == 0){
        if(command.args[1] == NULL){ // Checks for argument after cd 
            perror("cd: missing argument");
        }
        else{
            if(chdir(command.args[1]) != 0){ // Change directory and check for errors
                fprintf(stderr, "error %d: %s\n", errno, strerror(errno));
            }
        }
        return;
    }

    if (strcmp(command.cmd, "exit") == 0){ // Check for exit command
        exit(0);
    }

    pid_t pid = fork(); // creating child process

    if(pid < 0){
        fprintf(stderr, "error %d: %s\n", errno, strerror(errno));
        exit(1);
    }

    if(pid == 0){
        if(command.redirectInput){ // Check for input redirection
            int fd = open(command.inputFile, O_RDONLY); // Open the input file for reading and check for errors
            if(fd < 0){
                fprintf(stderr, "error %d: %s\n", errno, strerror(errno));
                exit(1);
            }
            dup2(fd, STDIN_FILENO); // Redirect standard input to the file descriptor of the opened file
            close(fd);
        }
        if(command.redirectOutput){ // Check for output redirection
            int fd = open(command.outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open the output file for writing, and create a new one if it doesn't exist, and check for errors
            if(fd < 0){
                fprintf(stderr, "error %d: %s\n", errno, strerror(errno));
                exit(1);
            }
            dup2(fd, STDOUT_FILENO); // Redirect standard output to the file descriptor of the opened file
            close(fd);
        }
        errno = 0;
        execvp(command.cmd, command.args); // Execute the command with arguments and check for errors
        fprintf(stderr, "error %d: %s\n", errno, strerror(errno));
        exit(1);
    }
    else{

        wait(NULL);
    }
}