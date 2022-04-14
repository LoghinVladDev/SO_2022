//
// Created by loghin on 4/4/22.
//

/**
 [MyCritSec #3]
Implementați problema secțiunii critice prezentată în cursul teoretic #5, în scenariul următor:
    Se consideră, drept resursă partajabilă de mai multe procese,
    un fișier binar cu baza de date folosită pentru gestiunea produselor dintr-un magazin.
    Înregistrările din acest fișier reprezintă perechi de forma:
        (cod_produs,stoc), unde
            cod_produs este un număr unic (i.e., apare o singură dată în baza de date) de tipul int,
            stoc este un număr de tipul float, reprezentând cantitatea din acel produs (
                exprimată în unitatea de măsură specifică pentru acel tip de produs, e.g. kilograme, litri, etc.
            ), disponibilă în acel magazin. Perechile de numere întregi și reale sunt reprezentate binar, nu textual,
            în fișierul rescpectiv!
        Asupra acestei baze de date se vor efectua operațiuni de actualizare a stocurilor de produse,
        conform celor descrise mai jos.

Scrieți un program C care să efectueze diverse operații de vânzare/cumpărare de produse,
la intervale variate de timp, operațiile fiind specificate,
într-un fișier text cu instrucțiuni, prin secvențe de forma:
    cod_produs +cantitate   și/sau   cod_produs -cantitate ,
        reprezentând cumpărarea și respectiv vânzarea cantității specificate din produsul având codul cod_produs specificat.

Pentru fiecare instrucțiune de cumpărare/vânzare din fișierul de instrucțiuni,
programul va căuta în fișierul resursă (i.e., baza de date) specificat înregistrarea cu
codul cod_produs specificat în instrucțiunea respectivă,
iar dacă există o astfel de înregistrare, atunci va actualiza valoarea stocului acelui produs în mod corespunzător,
dar NUMAI dacă această operație NU conduce la obținerea unei valori negative pentru stoc,
altfel va afișa un mesaj de eroare corespunzător.

Dacă nu există codul cod_produs specificat în instrucțiunea respectivă,
iar operația propusă este -cantitate,
programul va afișa un mesaj de eroare corespunzător și se va opri din procesarea
fișierului de instrucțiuni.
Iar dacă nu există codul cod_produs specificat în instrucțiunea respectivă,
însă operația propusă este +cantitate, atunci programul va adăuga o nouă înregistrare,
cu valoarea: (cod_produs,cantitate), în baza de date respectivă.

Cerințe:

Programul va accesa fișierul resursă în manieră cooperantă,
folosind lacăte în scriere pe durata de efectuare a fiecărei operațiuni de actualizare a stocului,
astfel încât să permită execuția simultană a două sau mai multor instanțe ale programului,
fără să apară efecte nedorite datorită fenomenelor de data race.

Cu alte cuvinte, ideea este de a executa simultan mai multe instanțe ale programului,
care vor accesa concurent același fișier și îl vor modifica,
coordonându-și însă accesul exclusiv (doar) la secțiunile modificate/scrise,
astfel încât să nu apară efecte nedorite datorită fenomenelor de data race.

(Indicație #1: pentru eficiența și ușurința de programare,
 perechile de numere de tipul int și float vor fi reprezentate binar în fișier,
 și nu textual, astfel încât fiecare număr întreg, respectiv real, să ocupe exact sizeof(int) octeți,
 respectiv sizeof(float) octeți. În acest fel,
 toate înregistrările din fișier vor avea lungime fixă (și anume, sizeof(int)+sizeof(float) octeți),
 ceea ce va ușura mult prelucrarea lor.)

Lacătele se vor pune numai pe porțiunea de fișier strict necesară și numai pe durata minimă necesară
 (asemănător ca la versiunea 4 a programului demonstrativ access prezentat în lecția practică despre lacăte pe fișiere).

(Indicație #2: se vor folosi apelurile de sistem open(), read(), write(), close() și respectiv fcntl()
 pentru punerea de lacăte pe porțiunile minimale, strict necesare, din fișierul de lucru,
 pe care lucrează la un moment dat o instanță a programului, iar blocajele vor fi păstrate
 doar pe durata minimă de timp necesară.)

Operațiile de actualizare vor fi implementate astfel
 încât să afișeze pe ecran mesaje explicative despre ceea ce se execută,
 fiecare mesaj fiind prefixat de PID-ul procesului ce execută respectiva operație de actualizare
 (pentru a putea face distincție între procesele, i.e. instanțele de execuție paralelă ale programului,
 ce afișează câte ceva).
 De asemenea, se va introduce câte o scurtă pauză (sub 1 secundă) între orice două operații
 de actualizare realizate succesiv de program.

Se va pregăti un mediu pentru testare, compus din: programul executabil,
 câte un fișier de instrucțiuni pentru fiecare instanță a executabilului lansată în execuție,
 fișierul asupra căruia se vor opera modificările,
 precum și un script bash care să lanseze în execuții paralele programul
 cu parametrii corespunzători (câte un fișier de instrucțiuni), adică o lansare pentru test ar putea fi de forma:

        UNIX> ./prg-sc3 depozit.bin instr1.txt & ./prg-sc3 depozit.bin instr2.txt & ./prg-sc3 depozit.bin instr3.txt & ...

        (Indicație #3: pentru a scrie scriptul bash pomenit mai sus,
        puteți să luați scriptul de la exercițiul rezolvat [Run SPMD programs] din
        Laboratorul #5 și să-l modificați astfel încât să poată transmite
        doi parametri către instanțele jobului SPMD pe care-l creează:
            primul parametru ar fi același pentru toate instanțele, i.e. numele fișierului cu stocuri,
            iar al doilea parametru ar fi specific pentru fiecare instanță, i.e. numele fișierului de instrucțiuni pentru instanța respectivă.
            Astfel adaptat, scriptul va putea fi invocat printr-o linie de comandă de forma:

        UNIX> ./RunMySPMD.sh ./prg-sc3 10 depozit.bin instr1.txt instr2.txt ... instr10.txt

        Evident, în loc de 10, se va putea apela cu un număr întreg pozitiv oarecare.)
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_INSTRUCTIONS 128

struct Instruction {
    int     productID;
    float   quantity;
};

struct InstructionSet {
    int                 count;
    struct Instruction  instructions [ MAX_INSTRUCTIONS ];
};

struct InstructionSet instructionSet;

void parseInstructionSet ( char const * fileName );
void executeInstructions ( struct InstructionSet const *, char const * depositFileName );

int main ( int argumentCount, char ** arguments ) {

    if ( argumentCount != 3 ) {
        fprintf (
                stdout,
                "Incorrect Usage. Required : ./MyCritSec3 <database_file> <instructions_file>\n"
        );

        return 0;
    }

    parseInstructionSet ( arguments[2] );
    executeInstructions ( & instructionSet, arguments[1] );
}

void parseInstructionSet ( char const * fileName ) {

    FILE *  instructionsFile = fopen ( fileName, "r" );
    char    instructionLine [ 1024 ];

    while ( ! feof ( instructionsFile ) ) {
        fgets ( instructionLine, 1024, instructionsFile );

        char * pSpace           = strchr ( instructionLine, ' ' );
        char * pProductID       = instructionLine;
        char * pProductQuantity = pSpace + 1;

        * pSpace = '\0';

        instructionSet.instructions [ instructionSet.count ++ ] = (struct Instruction) {
                .productID  = strtol ( pProductID, NULL, 10 ),
                .quantity   = strtol ( pProductQuantity, NULL, 10 )
        };
    }
}

void executeInstructions ( struct InstructionSet const * pInstructionSet, char const * depositFileName ) {

    int databaseFile = open ( depositFileName, O_RDWR | O_CREAT );

    for ( int i = 0; i < pInstructionSet->count; ++ i ) {

        int     readByteCount = 0;
        bool    productFound = false;

        do {

            int                         productID;
            float                       productQuantity;
            struct Instruction  * const pCurrentInstruction = & pInstructionSet->instructions[i];

            readByteCount = read ( databaseFile, & productID, sizeof ( int ) );
            readByteCount = read ( databaseFile, & productQuantity, sizeof ( float ) );

            if ( pCurrentInstruction->productID == productID ) {
                struct flock fileLock;

                lseek ( databaseFile, - (int) ( sizeof ( int ) + sizeof ( float ) ), SEEK_SET );

                fileLock.l_type     = F_WRLCK;
                fileLock.l_whence   = SEEK_CUR;
                fileLock.l_len      = sizeof ( int ) + sizeof ( float );
                fileLock.l_start    = 0;

                fcntl ( depositFileName, F_SETLKW, & fileLock );

                if ( productQuantity > pCurrentInstruction->quantity ) {
                    productQuantity += pCurrentInstruction->quantity;
                } else {
                    printf ( "Not enough quantity for product %d\n", pCurrentInstruction->productID );
                }

                lseek ( databaseFile, sizeof ( int ), SEEK_CUR );
                write ( databaseFile, & productQuantity, sizeof ( float ) );

                productFound = true;

                fileLock.l_type     = F_UNLCK;
                fileLock.l_whence   = SEEK_CUR;
                fileLock.l_len      = sizeof ( int ) + sizeof ( float );
                fileLock.l_start    = - (int) ( sizeof ( int ) + sizeof ( float ) );

                fcntl ( depositFileName, F_SETLKW, & fileLock );
            }

        } while ( readByteCount > 0 );
    }
}
