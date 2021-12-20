#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <limits.h>
#include <ctype.h>

void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

void printError(){
    char error_message[30] = "An error has occurred\n";
    write(STDOUT_FILENO, error_message, strlen(error_message));
}

void myExit(char** comms){
    if (comms[1] != NULL){
        printError();
        return;
    }
    exit(0);
} // WORKING

void myPWD(char** comms)
{
    if (comms[1] != NULL){
        printError();
        return;
    }
    char* cwd;
    char cwd_buff [4096 + 1];
    cwd = getcwd (cwd_buff, 4096 + 1);
    if (cwd != NULL){
        myPrint(cwd);
        myPrint("\n");
    } else {
        printError();
    }
}

void myCD(char** comms){
    if (comms[2] != NULL){
        printError();
        return;
    }
    if (comms[1] == NULL)
    {
        if (chdir(getenv("HOME")) != 0)
        {
            printError();
        }
    }
    else
    {
        if (chdir(comms[1]) != 0)
        {
            printError();
        }
    }
}
// Return 1 if input is within 512 exlcuding newline
// If input is too long, prints input
int withinBuffer (char* buff, FILE* file)
{
    // F INPUT EXCEEDS BUFFER
        if (buff [strlen (buff) - 1] != '\n')
        {
            if (file == stdin)
            {
                myPrint(buff);
            }
            // PRINTS OUT BUFFER
            int c = getc(file);
            // PRINT OUT WHAT REMAINS IN STDIN (not sure if supposed to just discard this)
            while (c != '\n' && c != EOF )
            {
                    myPrint ((char*) &c);
                    c = getc(file);  //Next 
            }
                
            myPrint ("\n");
            printError ();
            return 0;
        }
        else
        {
            return 1;
        }
        
}
// Separates a command and its arguements
// Executes command with arguements
void parseExecute (char* line)
{
    char** args = malloc (256 * sizeof (char*));
    char* arg;
    int i = 0;  //counter
    args[1] = NULL; // FOR SOME REASON ARGS [1] always comes in with a strange value
    arg = strtok_r (line, " \t", &line); //Get first word
    char* cmd  = arg; // First word is command
    while (arg != NULL) // While there are still words
    {
        args[i] = arg;
        arg = strtok_r (NULL, " \t", &line);
        i++;
    }
    if (cmd == NULL){} // If empty, do nothing
    else if (strcmp (cmd, "exit") == 0) // If base command, call directly
    {
         myExit (args);
    }
    else if (strcmp (cmd, "cd") == 0)
    {
        myCD (args);
    }
    else if (strcmp (cmd, "pwd") == 0)
    {
        myPWD (args);
    }
    else 
    {
        pid_t cpid, ppid;
        cpid = fork (); // Create a new process
        if (cpid == -1) // If issue forking
        {
            printError();
            exit (1);
        }
        else if (cpid == 0) //CHILD
        {
            if (execvp (cmd, args) == -1) // EXECUTE COMMAND 
            {
                printError();
                exit (1); // Closes child thread
            }
        }
        else // PARENT
        {
            int status;
            do {
                ppid = waitpid(cpid, &status, WUNTRACED);
                if (ppid == -1)
                {
                    printError();
                    exit (1);
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        } //LEARNED ABOUT WIP FROM HERE: https://pubs.opengroup.org/onlinepubs/9699919799/functions/wait.html
    } 
    while (i != 0)
    {
        args[i] = NULL;
        i--;
    }
    free (args); 
}
// Separates lines at at each newline or 
// Then sends them to parseExecute
void splitLines (char* input)
{
    char* line;
    line = strtok_r (input, ";\n", &input);
    while (line != NULL)
    {
        // printf ("Line: _%s_ \n", line);
        //printf ("Input: _%s_ \n", input);
        parseExecute (line);
        line = strtok_r (NULL, ";\n", &input);
    }
}
//for owen 
// Checks to see if a string only contains spaces and tabs or null
int isEmpty (char* str){
    int len = strlen(str);
    /*if (str[0]=='\n'){
        return 1;
    }*/
    for (int i = 0; i<len; i++){
        if((str[i]!=' ') && (str[i]!='\t') && str[i]!='\n'){
            return 0;
        }
    }
    return 1;
}
int main(int argc, char *argv[]) 
{
    // Next steps: Implement batch commands
    // If argc = 2 -> batch
    // If argc = 1 -> interactive
    // Else -> Throw error
    // Batch needs to read the file inputs line by line fgets(xxx, xx, fp)
    // and print out the commands as they get executed (this can be
    // implemented by modifying previous functions)
    // Blank commands are to be ignored
    //how to print from brief: write(STDOUT_FILENO, cmdline, strlen(cmdline));
    unsigned int batch;
    FILE* input; 
    if(argc == 2){
        batch = 1;
        input = fopen(argv[1], "r");
        //write(STDOUT_FILENO, "batch\n", strlen("batch\n"));
    } else if (argc == 1){
        batch = 0;
        input = stdin;
    } else {
        input = NULL;
        printError();
        exit(1);
    }
    char cmd_buff[514];
    char *pinput;
    if (input){     //check valid input source
        while (1) {
            if (!batch){    //interactive
            myPrint("myshell> ");
            }
            pinput = fgets(cmd_buff, 514, input);
            //myPrint("got input");
            if (!pinput) // Input is not null
            {            
                exit(0);
            }
            if (batch){
                //myPrint("should be printing commands");
                if (!(isEmpty(cmd_buff))){
                    //myPrint("{");
                    myPrint(cmd_buff);
                    //myPrint("}");
                }
            }
            if (withinBuffer(cmd_buff, input)) // Within bounds
            {
                splitLines (cmd_buff);
            // splitLines breaks up each user input into command blocks
            // then it feeds those command blocks one at a type to parseExecute
            // which breaks up command blocks into command and arugements
            // and executes them
            }
        }
        if (batch){
            fclose(input);
        }
    } else {
        printError();
        exit(1);
    }
    
}

/* Test strings
char* fiveHundredAndTwelveBlanks = "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ";
printf ("%li", strlen (fiveHundredAndTwelveBlanks));
char* fiveHundredAndThirteenBlanks = "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ";
printf ("%li", strlen (fiveHundredAndThirteenBlanks));
*/
