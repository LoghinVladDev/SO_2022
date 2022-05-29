#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#define READ 0
#define WRITE 1

#define READ_BUFFER_SIZE 1024


int parentToChild[2];
int childToParent[2];

void child () {
    close (parentToChild[WRITE]);
    close (childToParent[READ]);

    int length;
    read ( parentToChild[READ], & length, sizeof ( int ) );

    char * fileName = (char *) malloc ( length * sizeof(char) );
    read ( parentToChild[READ], fileName, length * sizeof (char) );

//    printf("Child Received : %s\n", fileName);

    FILE * inputFile = fopen ( fileName, "r" );

    if ( inputFile == NULL ) {
        close (parentToChild[READ]);
        close (childToParent[WRITE]);

        printf("File does not exist.\n");

        return;
    }

    int linePending = 0;

    char fileBuffer [READ_BUFFER_SIZE];
    while ( ! feof ( inputFile ) ) {
        fgets ( fileBuffer, READ_BUFFER_SIZE, inputFile );

        linePending = 1;

        write ( childToParent[WRITE], & linePending, sizeof ( int ) );
        write ( childToParent[WRITE], fileBuffer, READ_BUFFER_SIZE );
    }

    linePending = 0;
    write ( childToParent[WRITE], & linePending, sizeof(int) );

    fclose ( inputFile );

    free ( fileName );

    close (parentToChild[READ]);
    close (childToParent[WRITE]);
}

void parent (char * fileName) {
    close (parentToChild[READ]);
    close (childToParent[WRITE]);

    int length = strlen ( fileName ) + 1;
    write ( parentToChild[WRITE], & length, sizeof ( int ) );
    write ( parentToChild[WRITE], fileName, length * sizeof (char) );

    int linePending = 1;
    char fileBuffer [READ_BUFFER_SIZE];

    while ( linePending == 1 ) {

        int readCount = read ( childToParent [READ], & linePending, sizeof (int));

        if ( linePending == 0 || readCount == 0 ) {
            break;
        }

        read ( childToParent[READ], fileBuffer, READ_BUFFER_SIZE );

        printf("%s", fileBuffer);
    }

    close (parentToChild[WRITE]);
    close (childToParent[READ]);
}

int main ( int argumentCount, char ** argumentVector ) {

    if ( argumentCount != 2 ) {
        printf("File name expected as argument you dummy. \n");
        return 0;
    }

    pipe(parentToChild);
    pipe(childToParent);
    pid_t childPid = fork ();

    /// parinte -> processID > 0
    /// copil -> processID = 0

    if ( childPid == 0 ) {

        child();

        exit(0);

    } else {

        parent ( argumentVector[1] );

        waitpid ( childPid, NULL, 0 );

    }

    return 0;
}