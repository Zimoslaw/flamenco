#include "main.h"
#include "watek_glowny.h"

#define TANCERKA 0
#define GITARZYSTA 1

void mainLoop()
{
    srandom(rank);
    int tag;
    int changes = 0; // locznik zmian stanów

    int actorType = rank%2; // losowanie typu postaci

    if (actorType == TANCERKA) {
	    println("Tancerka gotowa");
    } else {
	    println("Gitarzysta gotowy");
    }

    while (stan != Finish) {
        if (changes<MAX_STATE_CHANGES) {
            // POCZĄTEK
            if (stan == Start) {
                changes++;

                // Dodaj siebie do kolejki
                putProcess(&processQueue, rank, lamport, actorType);

                println("Wysyłam swoje dane do wszystkich")

                // Wysyłanie SYNC do wszytkich
                for (int i = 0; i < size; i++) {
                    if (i == rank)
                        continue;
                    packet_t *pkt = malloc(sizeof(packet_t));
                    pkt->data = actorType;
                    pkt->ts = ++lamport;
                    sendPacket(pkt, i, SYNC);
                }
                changeState( Wait );
	        }
            // DODANIE PROCESU ADRESATA DO KOLEJKI
            if (stan == AddToQueue) {
                changes++;

                debug("Dodałem proces %d do kolejki", pakiet.src);

                // Sprawdzanie czy kolejka jest zakończona
                int c = 0;
                Process* p = processQueue;
                while(p != NULL) {
                    c++;
                    p = p->next;
                }
                if (c >= size) { // Tworzenie kolejki zakończone
                    println("Kolejka stworzona");
                    changeState( Wait );
                }

                changeState( Wait );
            }
            // CZEKANIE
            if (stan == Wait) {
                changes++;
            }
        } else {
	        changeState( Finish );
            
            Process* p = processQueue; 
            while (p != NULL) {
                debug("[%d, %d, %d], ", p->id, p->ts, p->type);
                p = p->next;
            }
        }
        sleep(SEC_IN_STATE);
    }
}
