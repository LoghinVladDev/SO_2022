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
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#define MAX_FILE_COUNT 128
#define MAX_RANGE_COUNT 128

struct Range {
    int start;
    int end;
};

struct CommandParameters {

    bool            cutByBytes;

    bool            cutByFields;
    bool            useCustomSeparator;
    char            customSeparatorValue;

    int             rangeCount;
    struct Range    ranges [ MAX_RANGE_COUNT ];

    int             inputFileCount;
    char    const * inputFilePaths [ MAX_FILE_COUNT ];

} parameters;

int parseArguments ( int argumentCount, char ** arguments );
int parseList ( char const * argument );
int parseDelim ( char const * argument );
void printHelp ();

bool isStringNumeric ( char const * string );

int cleanAndMergeRanges ();
int convertStringToInt ( char const * string, int * pInt );

void runCommand ();

/// test function
char * rangesToString () {
    static char asStr[8192];

    memset ( asStr, 0, 8192 );
    strcpy ( asStr, "[ " );

    for (int i = 0; i < parameters.rangeCount; ++ i ) {
        sprintf ( asStr + strlen ( asStr ), "%d-%d", parameters.ranges[i].start, parameters.ranges[i].end );
        if (i != parameters.rangeCount - 1 ) {
            strcat ( asStr, ", " );
        }
    }

    strcat ( asStr, " ]" );
    return asStr;
}

/// test function
char * pathsToString () {
    static char asStr[8192];

    memset ( asStr, 0, 8192 );
    strcpy ( asStr, "[ " );

    for (int i = 0; i < parameters.inputFileCount; ++ i ) {
        strcat (asStr, parameters.inputFilePaths[i] );
        if (i != parameters.inputFileCount - 1 ) {
            strcat ( asStr, ", " );
        }
    }

    strcat ( asStr, " ]" );
    return asStr;
}

int main ( int argumentCount, char ** arguments ) {

    if ( parseArguments ( argumentCount, arguments ) != 0 ) {
        return 1;
    }

    runCommand ();

//    fprintf (
//            stdout,
//            "Behavior : \n"
//            "\tcutByFields = %s\n"
//            "\tcutByBytes = %s\n"
//            "\tuseCustomSeparator = %s\n"
//            "\tcustomSeparatorValue = '%c'\n"
//            "\trangeCount = %d\n"
//            "\tranges = %s\n"
//            "\tinputFileCount = %d\n"
//            "\tinputFilePaths = %s\n",
//            parameters.cutByBytes == true ? "true" : "false",
//            parameters.cutByFields == true ? "true" : "false",
//            parameters.useCustomSeparator == true ? "true" : "false",
//            parameters.customSeparatorValue == '\0' ? ' ' : parameters.customSeparatorValue,
//            parameters.rangeCount,
//            rangesToString(),
//            parameters.inputFileCount,
//            pathsToString()
//    );

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

            if ( strcmp ( arguments[i], "-b" ) == 0 ) {
                expectedListForB                = true;
                parameters.cutByBytes            = true;

            } else if ( strcmp ( arguments[i], "-f" ) == 0 ) {
                expectedListForF                = true;
                parameters.cutByFields           = true;

            } else if ( strcmp ( arguments[i], "-d" ) == 0 ) {
                expectedDelimForC               = true;
                parameters.useCustomSeparator    = true;

            } else {
                parameters.inputFilePaths [ parameters.inputFileCount ++ ] = arguments[i];
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

    if (parameters.cutByFields && parameters.cutByBytes ) {
        fprintf (
                stderr,
                "Cannot cut by both bytes and fields\n"
        );

        return 1;
    }

    if (! parameters.cutByFields && ! parameters.cutByBytes ) {
        fprintf (
                stderr,
                "Must cut by either bytes or fields\n"
        );

        return 1;
    }

    bool anyFileInvalid = false;
    for ( int i = 0; i < parameters.inputFileCount; ++ i ) {
        struct stat fileStatistics;
        if ( lstat ( parameters.inputFilePaths[i], & fileStatistics ) == -1 ) {
            fprintf ( stdout, "Error : File '%s' does not exist / insufficient rights to open\n", parameters.inputFilePaths[i] );
            anyFileInvalid = true;
        }
    }

    if ( anyFileInvalid ) {
        return 1;
    }

    if ( parameters.rangeCount == 0 ) {
        fprintf ( stdout, "No ranges given for selection method\n" );
        return 1;
    }

    if ( parameters.inputFileCount == 0 ) {
        fprintf ( stdout, "No input files given\n" );
        return 1;
    }

    if ( expectedDelimForC ) {
        fprintf ( stdout, "Must specity custom delimiter if -d is used\n" );
        return 1;
    }

    if ( parameters.cutByBytes && parameters.useCustomSeparator ) {
        fprintf ( stdout, "Custom separator can only be used with -f option\n" );
        return 1;
    }

    if ( cleanAndMergeRanges () == 1 ) {
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
                currentRange.end    = INT32_MAX;
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

        parameters.ranges[ parameters.rangeCount ++ ] = currentRange;

        pCurrentSegment = strtok ( NULL, "," );
    }

    free ( listOfRanges );
    return 0;
}

int parseDelim ( char const * argument ) {
    parameters.customSeparatorValue = argument[0];
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

bool isInvalidRange ( struct Range r ) {
    return r.start == -1 && r.end == -1;
}

bool rangesIntersect ( struct Range a, struct Range b ) {
    return
        a.start >= b.start && a.start <= b.end ||
        a.end   >= b.start && a.end   <= b.end;
}

int min ( int a, int b ) {
    if ( a < b )
        return a;
    return b;
}

int max ( int a, int b ) {
    if ( a > b )
        return a;
    return b;
}

void mergeRange ( struct Range * pDest, struct Range src ) {
    pDest->start    = min ( pDest->start, src.start );
    pDest->end      = max ( pDest->end, src.end );
}

void invalidateRange ( struct Range * pRange ) {
    pRange->start   = -1;
    pRange->end     = -1;
}

int cleanAndMergeRanges () {

    for ( int i = 0; i < parameters.rangeCount; ++ i ) {

        if ( parameters.ranges[i].end < parameters.ranges[i].start ) {
            fprintf ( stderr, "Invalid decreasing range [%d-%d] given\n", parameters.ranges[i].start, parameters.ranges[i].end );
            return 1;
        }
    }

    for ( int i = 0; i < parameters.rangeCount - 1; ++ i ) {
        if ( ! isInvalidRange(parameters.ranges[i]) ) {

            for ( int j = i + 1; j < parameters.rangeCount; ++j ) {
                if ( ! isInvalidRange(parameters.ranges[j]) ) {

                    if ( rangesIntersect ( parameters.ranges[i], parameters.ranges[j] ) ) {

                        mergeRange ( & parameters.ranges[i], parameters.ranges[j] );
                        invalidateRange ( & parameters.ranges[j] );
                    }
                }
            }
        }
    }

    struct Range newRanges [ MAX_RANGE_COUNT ];
    int newRangeCount = 0;

    for ( int i = 0; i < parameters.rangeCount; ++ i ) {
        if ( ! isInvalidRange( parameters.ranges[i] ) ) {
            newRanges[newRangeCount ++] = parameters.ranges[i];
        }
    }

    parameters.rangeCount = newRangeCount;
    for ( int i = 0; i < parameters.rangeCount; ++ i ) {
        parameters.ranges[i] = newRanges[i];
    }

    for ( int i = 0; i < parameters.rangeCount - 1; ++ i ) {
        for ( int j = i + 1; j < parameters.rangeCount; ++ j ) {
            if ( parameters.ranges[i].start > parameters.ranges[j].start ) {

                struct Range aux = parameters.ranges[i];
                parameters.ranges[i] = parameters.ranges[j];
                parameters.ranges[j] = aux;
            }
        }
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

bool isStringNumeric ( char const * string ) {
    char const * numbers = "0123456789";
    for ( int i = 0, length = strlen ( string ); i < length; ++ i ) {
        if ( strchr ( numbers, string[i] ) == NULL ) {
            return false;
        }
    }

    return true;
}


void runCutByBytes ();
void runCutByFields ();

void runCommand () {

    if ( parameters.cutByBytes ) {
        runCutByBytes ();
    }

    runCutByFields ();
}

void runCutByBytes () {

    for ( int i = 0; i < parameters.inputFileCount; ++ i ) {
        FILE  * pFile = fopen ( parameters.inputFilePaths[i], "r" );
        char    fileLine [ 8192 ];

        fgets ( fileLine, 8192, pFile );

        while ( ! feof ( pFile ) ) {

            int lineLength = (int) strlen ( fileLine ) - 1;

            for ( int j = 0; j < parameters.rangeCount; ++ j ) {
                for ( int k = parameters.ranges[j].start - 1; k < parameters.ranges[j].end; ++k ) {
                    if ( k < lineLength ) {
                        fprintf(stdout, "%c", fileLine[k]);
                    }
                }
            }

            fprintf ( stdout, "\n" );
            fgets ( fileLine, 8192, pFile );
        }
    }
}

void runCutByFields () {

    for ( int i = 0; i < parameters.inputFileCount; ++ i ) {
        FILE  * pFile = fopen ( parameters.inputFilePaths[i], "r" );
        char    fileLine [ 8192 ];
        char    delim [2] = " ";

        if ( parameters.useCustomSeparator ) {
            delim[0] = parameters.customSeparatorValue;
        }

        fgets ( fileLine, 8192, pFile );

        while ( ! feof ( pFile ) ) {

            int fieldIndex  = 1;
            char * pField   = strtok ( fileLine, delim );

            while ( pField != NULL ) {

                char * pNewLine = strchr ( pField, '\n' );
                if ( pNewLine != NULL ) {
                    * pNewLine = '\0';
                }

                bool isInRange = false;
                for ( int j = 0; j < parameters.rangeCount; ++ j ) {
                    if ( parameters.ranges[j].start <= fieldIndex && fieldIndex <= parameters.ranges[j].end ) {
                        isInRange = true;
                        break;
                    }
                }

                if ( isInRange ) {
                    fprintf ( stdout, "%s ", pField );
                }

                pField = strtok ( NULL, delim );
                fieldIndex ++;
            }

            fprintf ( stdout, "\n" );
            fgets ( fileLine, 8192, pFile );
        }
    }
}
