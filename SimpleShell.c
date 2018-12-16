#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define N 10
#define M 100


int inputRedirectionFlag = 0;
int outputRedirectionFlag = 0;
int pipeFlag = 0;
char* inputFile = NULL;
char* outputFile = NULL;

// a function that removes the end of line from the fgets() function
void removeEndOfLine(char line[])
{

    int i = 0;
    while (line[i]!='\n') i++;
    line[i] = '\0';


}

// a function to read the line
void readLine(char line[])
{
	printf("%s","sish:> ");    // Shell style:    sish:> command_line
    //a built-in function that can take input as a line and store it as 1 word per index in an array
    /*its arguments are (the line it will read-
    the maximum number of chars per word of that line-
    the file it reads from)-
    Reading from console, so we use stdin
    */
	

    char* ret = fgets(line,M,stdin);

    removeEndOfLine(line);

    // exits program if "exit" is entered or CTRL+D are pressed
    if(!strcmp(line,"exit") ||ret == NULL) exit(0);


}


// a function that extracts the keywords from the line and put it in the args
int processLine(char* temp[], char line[])
{
    int i = 0;
    // a built-in function that reads the line and divides it whenever it finds a space
    temp[i] = strtok(line, " ");

    while(temp[i]!= NULL)
    {
        i++;
        temp[i] = strtok(NULL, " ");
    }


    return 1;


}
// a function that checks whether or not we are piping or redirecting

int pipeOrRedirect(char* temp[])
{
    int i = 0;


    while(temp[i]!= NULL)
    {
        if (strcmp(temp[i],">")==0)
        {
            outputRedirectionFlag = 1;
            outputFile = temp[i+1];

            return i;
        }
        if (strcmp(temp[i],"<")==0)
        {
            inputRedirectionFlag = 1;
            inputFile = temp[i+1];
            return i;
        }

         if (strcmp(temp[i],"|") ==0 )
        {
            pipeFlag = 1;

            return i;
        }
        i++;
    }

    return i;

}
// checks if the line is in correct form
int checkLine(char* temp[])
{
    int i = 0;
    int pipeCount = 0;
    int outputRedirectionCount=0;
    int inputRedirectionCount=0;
    int totalCount = 0;

    if(temp[i] ==  NULL) // if you press enter without entering anything
    {
        printf("Enter a command, please. \n");
        return 1;
    }

    while(temp[i]!= NULL)
    {

        if(strcmp(temp[i],">")==0) outputRedirectionCount++;
        if(strcmp(temp[i],"<")==0) inputRedirectionCount++;
        if(strcmp(temp[i],"|")==0) pipeCount++;


        i++;
    }
    totalCount = inputRedirectionCount + outputRedirectionCount + pipeCount;
    if(totalCount > 1) // if the total count is more than 1, it means the user is piping several times, or piping while redirecting, and that's wrong
    {
        printf("Error : Only 1 redirection or pipe allowed per line. ?\n");
        temp[0] = NULL;

    }

}



// a function to handle the input, both args and line
int readParseLine(char* args[], char line[], char* pipeArgs[])
{
    char* temp[N];
    int pos;
    int i = 0;
    int j = 0;
    readLine(line);
    processLine(temp,line);
    checkLine(temp);

    pos = pipeOrRedirect(temp);


    while(i<pos)
    {
        args[i] = temp[i];
        i++;
    }
        args[i] = NULL;

        
        if(pipeFlag==1) // means a command was entered, then a piping identifier was found
    {
        while(temp[i]!= NULL) // didn't get to the end of the array yet
        {	
			i++;
            pipeArgs[j] = temp[i];
            j++;
        }


    }

    return 1;

}

void pipeHandler(char* args[], char* pipeArgs[],int pipefd[])
{
    int pid;
    int i;

    pid = fork();

    if (pid == 0)
    {
        dup2(pipefd[1],1);
        close(pipefd[0]); // the parent doesn't need this end of the pipe
        execvp(args[0],args);
        perror(args[0]);
    }
    else
    {
        dup2(pipefd[0],0);
        close(pipefd[1]); // the child doesn't need tihs end of the pipe
        execvp(pipeArgs[0],pipeArgs);
        perror(pipeArgs[0]);
    }



}
int main()
{   // args are basically the commands you give the terminal
    char* args[N];
    // line is every word of the commands you give the terminal
    char line[M];
    // the piping arguments that show up in the command after the piping identifier
    char* pipeArgs[N];

    int pipefd[2]; // pipe file desriptor
    pipe(pipefd);
	
    while(readParseLine(args,line, pipeArgs))
    {
        pid_t pid=fork();

        /*      typedef int x;
                x y;
        */
        if(pid == 0)
        {
            // in child
           if(inputRedirectionFlag && inputFile!= NULL)
                dup2(open(inputFile,O_RDWR|O_CREAT,0777),0);


            if(outputRedirectionFlag && outputFile!= NULL)
                dup2(open(outputFile,O_RDWR|O_CREAT,0777),1);

            if(pipeFlag == 1)
            {
                pipeHandler(args, pipeArgs,pipefd);
                exit(0);
            }
            execvp(args[0],args);
        }
        else
        {
            // in parent
            waitpid(pid, 0);

            inputRedirectionFlag = 0;
            outputRedirectionFlag = 0;
            pipeFlag = 0;
            outputFile = NULL;
            inputFile = NULL;


        }
    }
    return 0;
}

/*

    List of commands :
    cd (directory)  -- doesn't work
    ls | head -3 
    ls
    ls > filename
    wc -l < filename
    exit = ctrl + d
    command --help ( this is built in )
*/
