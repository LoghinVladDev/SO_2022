//
// Created by loghin on 4/4/22.
//

/**
* [#1: The filter NoVocals]
Să se scrie un program C care primește de la linia de comandă numele a două fișiere, cu care va face următoarea procesare:
    va copia conținutul fișierului de intrare în cel de ieșire, eliminând vocalele întâlnite (majuscule și minuscule).
    În caz că fișierul de ieșire deja există, se va cere confirmare de suprascriere.
    Respectiv, va fi creat în cazul în care nu există, cu drepturi de citire și scriere doar pentru proprietar.

Cerință: se vor utiliza apelurile de sistem din API-ul POSIX pentru accesarea fișierelor.

(Indicație: printr-o singură parcurgere a fișierului de intrare, copiați fiecare caracter citit, aplicând transformarea cerută, în fișierul de ieșire.)
Cerință suplimentară:
    dacă de la linia de comandă se primește un singur nume de fișier,
    sau dacă numele fișierului de ieșire coincide cu numele celui de intrare,
    atunci se va trata, într-un mod adecvat, această situație (i.e., nu se mai face copiere, ci supra-scriere).
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#define OVERWRITE_PROMPT_REFUSED -2

int filterIntoOtherFile ( char const * readFilePath, char const * writeFilePath );
int filterAndOverrideInputFile ( char const * filePath );
int openWithOverridePrompt ( char const * writeFilePath );

bool isVowel ( char c );

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
        return filterIntoOtherFile ( arguments[1], arguments[2] );
    } else {
        return filterAndOverrideInputFile ( arguments[1] );
    }
}

int filterIntoOtherFile ( char const * readFilePath, char const * writeFilePath ) {

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

        if ( ! isVowel ( character ) ) {
            write ( outputFileDescriptor, & character, sizeof ( char ) );
        }

        readDataLength = read ( inputFileDescriptor, & character, sizeof ( char ) );
    }

    close ( inputFileDescriptor );
    close ( outputFileDescriptor );

    return 0;
}

bool isVowel ( char character ) {
    char const vowels [] = "AEIOUaeiou";

    return character != '\0' && strchr ( vowels, character ) != NULL;
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

int filterAndOverrideInputFile ( char const * filePath ) {

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

        if ( ! isVowel ( fileContents[i] ) ) {

            write ( outputFileDescriptor, & fileContents[i], sizeof ( char ) );
        }
    }

    free ( fileContents );
    close ( outputFileDescriptor );
    return 0;
}
