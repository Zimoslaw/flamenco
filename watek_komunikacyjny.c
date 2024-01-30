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

		incLamport(pakiet.ts);

        switch ( status.MPI_TAG ) {
	    case FINISH: 
            changeState( Finish );
	    break;
	    case SYNC: 
            debug("Dostałem pakiet SYNC od %d", pakiet.src);
			putProcess(&processQueue, pakiet.src, pakiet.ts, pakiet.data);
			changeState( AddToQueue );
	    break;
	    default:
			changeState( Finish );
	    break;
        }

		sleep(SEC_IN_STATE);
    }
}
