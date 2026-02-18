// Name(s): Christopher Sandoval, Luke Turpin
// Description:


//////// Some function ideas: ////////////
// Note: Some code is reflected in main that represents these functions,
// but it is up to you to determine how you want to organize your code.

/* 
    A function that causes the prompt to 
    display in the terminal
*/

/*
    A function that takes input from the user.
    It may return return the input to the calling statement or 
    store it at some memory location using a pointer.
*/ 

/*
    A function that parses through the user input.
    Consider having this function return a struct that stores vital
    information about the parsed instruction such as:
    - The command itself
    - The arguments that come after the command 
        Hint: When formatting your data, 
        look into execvp and how it takes in args.
    - Information about if a redirect was detected such as >, <, or |
    - Information about whether or not a new file 
        needs to be created and what that filename may be.
    

    Some helpful functions when doing this come from string.h and stdlib.h, such as
    strtok, strcmp, strcpy, calloc, malloc, realloc, free, and more

    Be sure to consider/test for situations when a backslash is used to escape the space char
    and when quotes are used to group together various tokens.
*/

/*
    A function that executes the command. 
    This function might take in a struct that represents the shell command.

    Be sure to consider each of the following:
    1. The execvp() function. 
        This can execute commands that already exist, that is, 
        you don't need to recreate the functionality of 
        the commands on your computer, just the shell.
        Keep in mind that execvp takes over the current process.
    2. The fork() function. 
        This can create a process for execvp to take over.
    3. cd is not a command like ls and mkdir. 
        cd is a toold provided by the shell, 
        so you WILL need to recreate the functionality of cd.
    4. Be sure to handle standard output redirect and standard input redirects here 
        That is, there symbols: > and <. 
        Pipe isn't required but could be a nice addition.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <fcntl.h>

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
	}

	exit(0);
}

void displayPrompt(){
    char cwd[1024]; 

    if (getcwd(cwd, sizeof(cwd)) != NULL) { // Get the current working directory
        printf("%s$ ", cwd); // Display the current working directory followed by a $ symbol
    } else {
        perror("getcwd() failed"); 
    }
}

char* getInput(){
    char *buffer = malloc(MAX_INPUT_SIZE); // Allocate memory for the input buffer
    if(!buffer){ // Checks for successful memory allocation
        perror("malloc() failed");
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