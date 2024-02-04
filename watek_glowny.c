#include "main.h"
#include "watek_glowny.h"

#define TANCERKA 0
#define GITARZYSTA 1

void mainLoop()
{
    srandom(rank);
    int tag;
    int changes = 0; // locznik zmian stanów

    Packet currentPacket; // obecnie odczytywany pakiet

    // losowanie typu postaci
    int actorType = random()%100;
    if (actorType >= 50)
        actorType = TANCERKA;
    else
        actorType = GITARZYSTA;


    if (actorType == TANCERKA) {
	    println("Tancerka gotowa");
    } else {
	    println("Gitarzysta gotowy");
    }

    while (stan != Finish) {
        if (changes<MAX_STATE_CHANGES) {

            // POBIERANIE DANYCH PAKIETU PIERWSZEGO W KOLEJCE
            if (stan != Start && packetQueue != NULL) {
                currentPacket = getFirstPacket(&packetQueue);

                switch(currentPacket.tag) {
                    case REQ_SYNC:
                        changeState( SendSync );
                    break;
                    case SYNC:
                        changeState( AddToQueue );
                    break;
                    default:
                        changeState( Wait );
                    break;
                }
            }

            // POCZĄTEK
            if (stan == Start) {
                changes++;

                println("Wysyłam żądanie o dane do wszystkich")

                // Wysyłanie REQ_SYNC do wszytkich
                for (int i = 0; i < size; i++) {
                    if (i == rank)
                        continue;
                    packet_t *pkt = malloc(sizeof(packet_t));
                    pkt->data = actorType;
                    pkt->ts = ++lamport;
                    sendPacket(pkt, i, REQ_SYNC);
                }
                changeState( Wait );
	        }
            // ODESYŁANIE INFORMACJ O SOBIE
            if (stan == SendSync) {
                changes++;

                packet_t *pkt = malloc(sizeof(packet_t));
                pkt->data = actorType;
                pkt->ts = ++lamport;
                sendPacket(pkt, currentPacket.src, SYNC);

                changeState( Wait );
            } 
            // DODANIE PROCESU ADRESATA DO KOLEJKI
            if (stan == AddToQueue) {
                changes++;

                putProcess(&processQueue, currentPacket.src, currentPacket.ts, currentPacket.data);
                debug("Dodałem proces %d do kolejki", currentPacket.src);

                // Sprawdzanie czy kolejka jest zakończona
                int c = 0;
                Process* p = processQueue;
                while(p != NULL) {
                    c++;
                    p = p->next;
                }
                if (c >= size - 1) { // Tworzenie kolejki zakończone
                    // Dodaj siebie do kolejki
                    putProcess(&processQueue, rank, lamport, actorType);
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
