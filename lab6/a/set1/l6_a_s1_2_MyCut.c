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
int parseList ( char const * argument );
int parseDelim ( char const * argument );
void printHelp ();

bool isStringNumeric ( char const * string );
int convertStringToInt ( char const * string, int * pInt );

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
            if ( parseList  ( arguments[i] ) != 0 ) {
                return 1;
            }

            expectedListForB    = false;

        } else if ( expectedListForF ) {
            if ( parseList ( arguments[i] ) != 0 ) {
                return 1;
            }

            expectedListForF    = false;

        } else {
            if ( parseDelim ( arguments[i] ) != 0 ) {
                return 1;
            }

            expectedDelimForC   = false;

        }
    }

    if ( behaviour.cutByFields && behaviour.cutByBytes ) {
        fprintf (
                stderr,
                "Cannot cut by both bytes and fields\n"
        );

        return 1;
    }

    if ( ! behaviour.cutByFields && ! behaviour.cutByBytes ) {
        fprintf (
                stderr,
                "Must cut by either bytes or fields\n"
        );

        return 1;
    }

    return 0;
}

int parseList ( char const * argument ) {
    size_t argumentLength = strlen ( argument );
    char * listOfRanges = (char *) malloc ( argumentLength + 1 );
    memcpy ( listOfRanges, argument, argumentLength + 1 );

    char * pCurrentSegment = strtok ( listOfRanges, "," );

    while ( pCurrentSegment != NULL ) {

        size_t segmentLength    = strlen ( pCurrentSegment );
        char * pDash            = strchr ( pCurrentSegment, '-' );

        struct Range currentRange;

        if ( pDash == NULL ) {

            int number;
            if ( convertStringToInt ( pCurrentSegment, & number ) != 0 ) {
                free ( listOfRanges );
                return 1;
            }

            currentRange.start  = number;
            currentRange.end    = number;

        } else {
            int dashPos = (int) (pDash - pCurrentSegment);

            if ( dashPos == 0 ) {
                int number;
                if ( convertStringToInt ( pCurrentSegment + 1, & number ) != 0 ) {
                    free ( listOfRanges );
                    return 1;
                }

                currentRange.start  = 0;
                currentRange.end    = number;

            } else if ( dashPos == segmentLength - 1 ) {
                int number;
                * pDash = '\0';

                if ( convertStringToInt ( pCurrentSegment, & number ) != 0 ) {
                    free ( listOfRanges );
                    return 1;
                }

                currentRange.start  = number;
                currentRange.end    = -1;
            } else {

                char * pBeforeDash  = pCurrentSegment;
                char * pAfterDash   = pDash + 1;
                * pDash             = '\0';

                int leftNumber, rightNumber;

                if (
                        convertStringToInt ( pBeforeDash, & leftNumber ) != 0 ||
                        convertStringToInt ( pAfterDash, & rightNumber ) != 0
                ) {

                    free ( listOfRanges );
                    return 1;
                }

                currentRange.start  = leftNumber;
                currentRange.end    = rightNumber;
            }
        }

        behaviour.ranges[ behaviour.rangeCount ++ ] = currentRange;

        pCurrentSegment = strtok ( NULL, "," );
    }

    free ( listOfRanges );
    return 0;
}

int parseDelim ( char const * argument ) {
    behaviour.customSeparatorValue = argument[0];
    return 0;
}

int convertStringToInt ( char const * string, int * pInt ) {
    if ( ! isStringNumeric ( string ) ) {
        fprintf (
                stderr,
                "Unexpected symbol '%s', expected integer\n",
                string
        );

        return 1;
    }

    * pInt = (int) strtol ( string, NULL, 10 );

    if ( * pInt == 0 ) {
        fprintf (
                stderr,
                "All ranges are numbered from 1'\n"
        );

        return 1;
    }

    return 0;
}

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
