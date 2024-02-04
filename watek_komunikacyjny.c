#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;

    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=Finish ) {
		debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		incLamport(pakiet.ts); // inkrementacja zegara lamporta
		putPacket(&packetQueue, pakiet.src, status.MPI_TAG, pakiet.ts, pakiet.data); // dodaj pakiet do kolejki

        switch ( status.MPI_TAG ) {
	    case FINISH: 
            changeState( Finish );
	    break;
		case REQ_SYNC:
			debug("Dostałem pakiet REQ_SYNC od %d", pakiet.src);
		break;
	    case SYNC:
            debug("Dostałem pakiet SYNC od %d", pakiet.src);
	    break;
	    default:
			changeState( Finish );
	    break;
        }

		sleep(SEC_IN_STATE);
    }
}
