#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

struct tagNames_t{
    const char *name;
    int tag;
} tagNames[] = { { "finish", FINISH}, { "SYNC", SYNC}};

const char const *tag2string( int tag )
{
    for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) {
	if ( tagNames[i].tag == tag )  return tagNames[i].name;
    }
    return "<unknown>";
}
/* tworzy typ MPI_PAKIET_T
*/
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    int       blocklengths[NITEMS] = {1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

/* opis patrz util.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string( tag), destination);
    if (freepkt) free(pkt);
}

void changeState( state_t newState )
{
    debug("stan: %d", newState);
    pthread_mutex_lock( &stateMut );
    if (stan==Finish) { 
	pthread_mutex_unlock( &stateMut );
        return;
    }
    stan = newState;
    pthread_mutex_unlock( &stateMut );
}

// Inkrementacja zegara lamporta
void incLamport(int ts)
{
    if (ts > lamport)
        lamport = ts;
}

/* Dodanie procesu do kolejki
( kolejka, id procesu, timestamp ) */
void putProcess(Process** queue, int pid, int timestamp, int type)
{
    Process* p = (Process*) calloc(1, sizeof(Process));
    p->id = pid;
    p->ts = timestamp;
    p->type = type;

    if (*queue == NULL) {
        *queue = p;
        return;
    }
    
    Process* current = *queue;

    if (p->id < current->id) {
        *queue = p;
        p->next = current;
        return;
    } else if (p->id == current->id && p->ts > current->ts) {
        *queue = p;
        p->next = current;
        return;
    }
    while (current->next != NULL && p->id > current->next->id) {
        current = current->next;
    }
    while (current->next != NULL && p->id == current->next->id && p->ts < current->next->ts) {
        current = current->next;
    }
    p->next = current->next;
    current->next = p;

    return;
}

// Czyszczenie kolejki
void clearProcessQueue(Process** queue)
{
    Process* current = *queue;
    Process* next = current->next;

    while(current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    *queue = NULL;
}
