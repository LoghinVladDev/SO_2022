//
// Created by loghin on 4/4/22.
//

/**
[#2: The command MyCut]
Să se scrie un program C ce implementează comanda cut, inclusiv cu opțiunile -b, -f și -d.
    Se va permite precizarea de argumente multiple de tip nume de fișiere,
    în linia de comandă a programului, pentru procesare.

Cerință: se vor utiliza apelurile de sistem din API-ul POSIX pentru accesarea fișierelor.

(Indicație: încercați să simulați cât mai exact comportamentul comenzii cut.)
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_FILE_COUNT 128
#define MAX_RANGE_COUNT 128

struct Range {
    int start;
    int end;
};

struct CommandBehaviour {

    bool            cutByBytes;

    bool            cutByFields;
    bool            useCustomSeparator;
    char            customSeparatorValue;

    int             rangeCount;
    struct Range    ranges [ MAX_RANGE_COUNT ];

    int             inputFileCount;
    char    const * inputFilePaths [ MAX_FILE_COUNT ];

} behaviour;

int parseArguments ( int argumentCount, char ** arguments );
int parseListForBytes ( char const * argument );
int parseListForFields ( char const * argument );
int parseDelim ( char const * argument );
void printHelp ();

int main ( int argumentCount, char ** arguments ) {

    if ( parseArguments ( argumentCount, arguments ) != 0 ) {
        return 1;
    }

    return 0;
}

int parseArguments ( int argumentCount, char ** arguments ) {

    bool expectedListForB   = false;
    bool expectedListForF   = false;
    bool expectedDelimForC  = false;

    if ( argumentCount == 1 ) {
        fprintf (
                stdout,
                "cut: you must specify a list of bytes, characters, or fields\n"
                "Try 'cut --help' for more information."
        );

        return 1;
    }

    for ( int i = 1; i < argumentCount; ++ i ) {
        if ( strcmp ( arguments[i], "--help" ) == 0 ) {
            printHelp ();
            return 0;
        }

        if ( ! expectedListForB && ! expectedListForF && ! expectedDelimForC ) {

            if ( strcmp ( arguments[i], "--b" ) == 0 ) {
                expectedListForB                = true;
                behaviour.cutByBytes            = true;

            } else if ( strcmp ( arguments[i], "--f" ) == 0 ) {
                expectedListForF                = true;
                behaviour.cutByFields           = true;

            } else if ( strcmp ( arguments[i], "--d" ) == 0 ) {
                expectedDelimForC               = true;
                behaviour.useCustomSeparator    = true;

            } else {
                behaviour.inputFilePaths [ behaviour.inputFileCount ++ ] = arguments[i];
            }

        } else if ( expectedListForB ) {
            if ( parseListForBytes ( arguments[i] ) != 0 ) {

            }

            expectedListForB    = false;

        } else if ( expectedListForF ) {
            parseListForFields ( arguments[i] );
            expectedListForB    = false;

        } else {
            parseDelim ( arguments[i] );
            expectedDelimForC   = false;

        }
    }

    return 0;
}

int parseListForBytes ( char const * argument ) {

}

int parseListForFields ( char const * argument );
int parseDelim ( char const * argument );

void printHelp () {
    fprintf (
            stdout,
            "Usage: cut OPTION... [FILE]...\n"
            "Print selected parts of lines from each FILE to standard output.\n"
            "\n"
            "With no FILE, or when FILE is -, read standard input.\n"
            "\n"
            "Mandatory arguments to long options are mandatory for short options too.\n"
            "  -b, --bytes=LIST        select only these bytes\n"
            "  -d, --delimiter=DELIM   use DELIM instead of TAB for field delimiter\n"
            "  -f, --fields=LIST       select only these fields;  also print any line\n"
            "                            that contains no delimiter character, unless\n"
            "                            the -s option is specified\n"
            "\n"
            "Use one, and only one of -b, -c or -f.  Each LIST is made up of one\n"
            "range, or many ranges separated by commas.  Selected input is written\n"
            "in the same order that it is read, and is written exactly once.\n"
            "Each range is one of:\n"
            "\n"
            "  N     N'th byte, character or field, counted from 1\n"
            "  N-    from N'th byte, character or field, to end of line\n"
            "  N-M   from N'th to M'th (included) byte, character or field\n"
            "  -M    from first to M'th (included) byte, character or field\n"
    );
}
