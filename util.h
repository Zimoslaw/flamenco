#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;  

    int data;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3

// Typ procesu w kolejce
typedef struct Process
{
    int id;
    int ts;
    int type;
    struct Process* next;
} Process;

// Typ pakietu w kolejce
typedef struct Packet
{
    int src;
    int tag;
    int ts;
    int data;
    struct Packet* next;
} Packet;

/* Typy wiadomości */
#define FINISH 999
#define SYN 100
#define SYNACK 101
#define REQ_ROOM 102
#define IM_FIRST 103
#define ACK 104
#define RELEASE 105

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);
#endif
