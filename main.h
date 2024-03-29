#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "util.h"
/* boolean */
#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 1
#define MAX_LAMPORT 100

#define ROOT 0

extern int rank;
extern int size;
extern int lamport; // zegar lamporta
extern Process *processQueue; // Kolejka procesów
extern Packet *packetQueue; // kolejka pakietów
extern packet_t pakiet;
typedef enum {Start, Wait, SendSynAck, CheckQueue, RoomRequested, CheckIfFirst, First, InRoom, Release, Finish} state_t;
extern state_t stan;
extern state_t prevStan;
extern pthread_t threadKom, threadMon;

extern pthread_mutex_t stateMut;




/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
					   "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape. 
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów
                                            
*/
#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d] [t%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamport,  ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

// makro println - to samo co debug, ale wyświetla się zawsze
#define println(FORMAT,...) printf("%c[%d;%dm [%d] [t%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamport, ##__VA_ARGS__, 27,0,37);

void changeState( state_t );

void incLamport( int );

void putProcess( Process**, int, int, int );

void clearProcessQueue(Process**);

void putPacket( Packet**, int, int, int, int );

Packet getFirstPacket(Packet**);

#endif
