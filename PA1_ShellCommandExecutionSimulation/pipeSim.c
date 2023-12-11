// CS 307, Programming Assignment 1 
// due date 6th November, 2023
// by İpek Akkus-30800

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char*argv[]) {
    
    printf("I'm SHELL process with PID: %d - Main command is man diff | grep -i -A1 -m1 > output.txt \n", getpid());
    
    // Create file descriptor and the pipe to be used in communication 
    int fd[2]; // 0 --> read & 1 --> write in pipe
    int pipe(fd); 
    
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        return 1;
    }
    //printf("Pipe is validated\n"); // For DEBUG

    int pid1 = fork(); // First fork for the child 'man'
    //Check validity of the pid for first child
    if (pid1 == -1) {
        perror("Fork 1 failed"); 
        return 2;
    } else if (pid1 == 0) { // The first child process (MAN)
        printf("I'm MAN process with PID: %d - My command is: man diff \n", getpid());
        close(fd[0]); // closing unnecessary ends, prevents possible resource leaks
        // Redirect stdout to the write end of filde descriptor (fd[1]) 
        dup2(fd[1], STDOUT_FILENO);  // Output of the man command will be written to file descriptor
        close(fd[1]); // Close the original write end of the file descriptor after duplication
        
        // Execute 'man' command to display the manual page for 'diff'
        char* man_args[] = {"man", "diff", NULL};
        execvp("man", man_args);
        // The code after execlp will only run if there's an error
        perror("Exec failed");
        exit(1);
    
    } else { // Shell process 

        int pid2 = fork(); // Second fork from the shell for the child 'grep'
        // Check validity of the pid for the second child
        if (pid2 == -1) {
            perror("Fork 2 failed");
            return 2; // Different returns helps us to identify the reason of error
        } else if (pid2 == 0) { // The second child process (GREP)
            
            printf("I'm GREP command with PID: %d - My command is: grep -i -A1 -m1 \n", getpid());
            close(fd[1]); // Closing unnecessary ends, prevents possible resource leaks
            // Redirect stdin to the read end of filde descriptor (fd[0]), meaning that the output of the first child process is now here
            dup2(fd[0], STDIN_FILENO); 
            close(fd[0]); // Close the original read end of the file descriptor after duplication

            // Creates output.txt file
            const char *filename = "output.txt";
            int file = open(filename,  O_WRONLY | O_CREAT , S_IRUSR | S_IWUSR);   // Not O_APPEND as it appends on the file, which means the result differs in each execution of the program
            // Writes the output of grep process to output.txt
            dup2(file, STDOUT_FILENO); 

            // Execute 'grep' to search for the literal string "-i"
            char* grep_args[] = {"grep", " -i", "-A1", "-m1",   NULL}; // (IN man diff) SEARCHS FOR -i, TAKES 1 LINE AFTER (-A1) IT AND WHEN IT FINDS ANY MATCH, IT STOPS AS IT IS ALLOWED TO TAKE MAX 1 (-m1)
            execvp("grep", grep_args);
            // The code after execlp will only run if there's an error
            error("Exec failed");
            exit(1); 

        } else { // Shell process  
            // Not used wait, as it not guarantees the order of the waited child processes
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0); 

            printf("I am SHELL command with PID:%d - execution is completed, you can find the results in output.txt \n", getpid());
        }
    }
    return 0;
}