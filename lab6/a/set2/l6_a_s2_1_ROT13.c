//
// Created by loghin on 4/4/22.
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#define OVERWRITE_PROMPT_REFUSED -2

int ROTIntoOtherFile ( char const * readFilePath, char const * writeFilePath );
int ROTAndOverrideInputFile ( char const * filePath );
int openWithOverridePrompt ( char const * writeFilePath );

char getROTEquivalent (char c);

int main ( int argumentCount, char ** arguments ) {

    if ( argumentCount != 2 && argumentCount != 3 ) {
        fprintf (
                stderr,
                "Program requires at least one argument, "
                "at at most two. Either one file name, or two. "
                "Specified argument number : %d\n",
                argumentCount
        );

        return 1;
    }

    if ( argumentCount == 3 ) {
        return ROTIntoOtherFile ( arguments[1], arguments[2] );
    } else {
        return ROTAndOverrideInputFile ( arguments[1] );
    }
}

int ROTIntoOtherFile ( char const * readFilePath, char const * writeFilePath ) {

    int inputFileDescriptor = open ( readFilePath, O_RDONLY );

    if ( inputFileDescriptor == -1 ) {
        fprintf (
                stderr,
                "File at '%s' does not exist / insufficient rights for read\n",
                readFilePath
        );

        return 1;
    }

    int outputFileDescriptor = openWithOverridePrompt ( writeFilePath );

    if ( outputFileDescriptor == -1 ) {
        fprintf (
                stderr,
                "Insufficient rights to override File at '%s'\n",
                writeFilePath
        );

        return 1;
    }

    if ( outputFileDescriptor == OVERWRITE_PROMPT_REFUSED ) {
        return 0;
    }

    char character;
    size_t readDataLength = read ( inputFileDescriptor, & character, sizeof ( char ) );

    while ( readDataLength > 0 ) {

        character = getROTEquivalent(character);
        write ( outputFileDescriptor, & character, sizeof ( char ) );


        readDataLength = read ( inputFileDescriptor, & character, sizeof ( char ) );
    }

    close ( inputFileDescriptor );
    close ( outputFileDescriptor );

    return 0;
}

int openWithOverridePrompt ( char const * writeFilePath ) {

    struct stat fileStatistics;
    bool fileExists = stat ( writeFilePath, & fileStatistics ) != -1;

    if ( fileExists ) {
        fprintf (
                stdout,
                "File '%s' exists. Overwrite? [y/n] : ",
                writeFilePath
        );

        fflush ( stdout );

        char response;
        read ( 0, & response, sizeof ( char ) );

        if ( response == 'y' || response == 'Y' ) {
            return open ( writeFilePath, O_WRONLY | O_TRUNC );
        }

        return OVERWRITE_PROMPT_REFUSED;
    }

    return open ( writeFilePath, O_WRONLY | O_CREAT );
}

int ROTAndOverrideInputFile ( char const * filePath ) {

    int inputFileDescriptor = open ( filePath, O_RDONLY );

    if ( inputFileDescriptor == -1 ) {
        fprintf (
                stderr,
                "File at '%s' does not exist / insufficient rights for read\n",
                filePath
        );

        return 1;
    }

    long byteCount = lseek ( inputFileDescriptor, 0, SEEK_END );
    lseek ( inputFileDescriptor, 0, SEEK_SET );

    char * fileContents = (char *) malloc ( byteCount );

    read ( inputFileDescriptor, fileContents, byteCount );
    close ( inputFileDescriptor );

    int outputFileDescriptor = openWithOverridePrompt ( filePath );

    if ( outputFileDescriptor == -1 ) {
        fprintf (
                stderr,
                "Insufficient rights to override File at '%s'\n",
                filePath
        );

        free ( fileContents );
        return 1;
    }

    if ( outputFileDescriptor == OVERWRITE_PROMPT_REFUSED ) {

        free ( fileContents );
        return 0;
    }

    for ( int i = 0; i < byteCount; ++ i ) {

        fileContents [i] = getROTEquivalent(fileContents[i]);
        write ( outputFileDescriptor, & fileContents[i], sizeof ( char ) );
    }

    free ( fileContents );
    close ( outputFileDescriptor );
    return 0;
}

bool isUpperCase ( char c ) {
    return c >= 'A' && c <= 'Z';
}

bool isLowerCase ( char c ) {
    return c >= 'a' && c <= 'z';
}

char getROTEquivalent ( char c ) {
    int positionInAlphabet;

    if ( isUpperCase (c) )
        positionInAlphabet = c - 'A';
    else if ( isLowerCase(c) )
        positionInAlphabet = c - 'a';
    else
        return c;

    positionInAlphabet = ( positionInAlphabet + 13 ) % 26;

    if ( isUpperCase (c) )
        return (char)('A' + positionInAlphabet);
    else
        return (char)('a' + positionInAlphabet);
}
