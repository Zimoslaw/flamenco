#include "main.h"
#include "watek_glowny.h"

#define TANCERKA 0
#define GITARZYSTA 1
#define NUM_OF_TANCERKI 4
#define ROOMS 4

void mainLoop()
{
    srandom(rank);
    int tag;
    int changes = 0; // licznik zmian stanów
    int rooms[ROOMS] = {0, 0, 0, 0}; // sale
    int room = -1; // sala, w której się znajdujemy
    int RSN = 0; // losowa liczba, według której są sortowane procesy

    Packet currentPacket; // obecnie odczytywany pakiet

    // losowanie typu postaci
    int actorType = 1;
    if (rank < NUM_OF_TANCERKI)
        actorType = 0;


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
                    case REQ_ROOM:
                        changeState( RoomRequested );
                    break;
                    case IM_FIRST:
                        changeState( CheckIfFirst );
                    break;
                    case ACK:
                        changeState( InRoom );
                    break;
                    case RELEASE:
                        changeState( Release );
                    break;
                    default:
                        changeState( Wait );
                    break;
                }

                lamport++;
		        incLamport(currentPacket.ts); // inkrementacja zegara lamporta
            }

            // POCZĄTEK
            if (stan == Start) {
                changes++;

                // Losowanie liczby oznaczającej kolejność w kolejce
                RSN = rand();
                if (RSN % 2 == 0 && actorType == 1)
                    RSN++;
                if (RSN % 2 != 0 && actorType == 0)
                    RSN--;

                println("Wysyłam żądanie o dane do wszystkich");

                // Wysyłanie REQ_SYNC do wszytkich
                for (int i = 0; i < size; i++) {
                    if (i == rank)
                        continue;
                    packet_t *pkt = malloc(sizeof(packet_t));
                    pkt->data = RSN;
                    pkt->ts = ++lamport;
                    sendPacket(pkt, i, REQ_SYNC);
                }

                // Dodaj siebie do kolejki
                putProcess(&processQueue, rank, RSN, actorType);

                changeState( Wait );
	        }
            // ODESYŁANIE INFORMACJ O SOBIE
            if (stan == SendSync) {
                changes++;

                packet_t *pkt = malloc(sizeof(packet_t));
                pkt->data = RSN;
                pkt->ts = ++lamport;
                sendPacket(pkt, currentPacket.src, SYNC);

                changeState( Wait );
            } 
            // DODANIE PROCESU ADRESATA DO KOLEJKI
            if (stan == AddToQueue) {
                changes++;

                putProcess(&processQueue, currentPacket.src, currentPacket.data, currentPacket.data%2);
                debug("Dodałem proces %d do kolejki", currentPacket.src);

                // Sprawdzanie czy kolejka jest zakończona
                int c = 0;
                Process* p = processQueue;
                while(p != NULL) {
                    c++;
                    p = p->next;
                }
                if (c >= size) { // Tworzenie kolejki zakończone
                    println("Kolejka stworzona");

                    // Wyświetlanie lokalnej kolejki
                    Process* p = processQueue; 
                    while (p != NULL) {
                        debug("[%d, %d, %d], ", p->id, p->ts, p->type);
                        p = p->next;
                    }

                    if (actorType == 0)
                        changeState( CheckIfFirst );
                    else
                        changeState( Wait );
                }
                else
                    changeState( Wait );
            }
            // INKREMENTOWANIE ILOŚCI REQUESTÓW DO DANEJ SALI
            if (stan == RoomRequested) {
                changes++;

                rooms[currentPacket.data]++;

                changeState( Wait );
            }
            // SPRAWDZANIE CZY JESTEŚMY PIERWSZMI W KOLEJCE
            if (stan == CheckIfFirst) {

                Process *p = processQueue;
                while(p != NULL) {
                    if (p->type == actorType) {
                        if (p->id == rank) {
                            changeState( First ); // Jesteśmy pierwsi
                            break;
                        }
                        else {
                            changeState( Wait ); // Nie jesteśmy pierwsi
                            break;
                        }
                    }
                    p = p->next;
                }
            }
            // WYBIERANIE SALI, ROZSYŁANIE ŻĄDANIA O SALĘ
            // ODSYŁANIE ACK DO TANCERKI (GITARZYSTA)
            if (stan == First) {
                if (actorType == 0) {
                    println("Jestem pierwszą tancerką");
                    // Losowanie pokoju
                    room = 0;
                    do {
                        room = random()%ROOMS;
                    }
                    while(rooms[room] != 0);

                    // Wysyłanie REQ_ROOM do tancerek
                    println("Wysyłam żądania o %d salę", room);
                    Process *p = processQueue;
                    while(p != NULL) {
                        if (p->type == 0 && p->id != rank) {
                            packet_t *pkt = malloc(sizeof(packet_t));
                            pkt->data = room;
                            pkt->ts = ++lamport;
                            sendPacket(pkt, p->id, REQ_ROOM);
                        }
                        p = p->next;
                    }

                    // Wysyłanie IM_FIRST do tancerek
                    println("Wysyłam info, że jestem pierwsza do gitarzystów");
                    p = processQueue;
                    while(p != NULL) {
                        if (p->type == 1) {
                            packet_t *pkt = malloc(sizeof(packet_t));
                            pkt->data = room;
                            pkt->ts = ++lamport;
                            sendPacket(pkt, p->id, IM_FIRST);
                        }
                        p = p->next;
                    }

                    changeState( Wait );
                }
                if (actorType == 1) {
                    room = currentPacket.data;
                    // Odsyłanie ACK do naszej tancerki z pary
                    packet_t *pkt = malloc(sizeof(packet_t));
                    pkt->data = currentPacket.data;
                    pkt->ts = ++lamport;
                    sendPacket(pkt, currentPacket.src, ACK);

                    changeState( InRoom );
                }
            }
            // JESTEŚMY W SALI
            if (stan == InRoom) {

                println("Wchodzę do sali nr %d (moja para to %d)", room, currentPacket.src);
                
                if (actorType == 0) {
                    println("Wychodzę z sali nr %d", room);

                    // Wysyłanie RELEASE do wszytkich
                    for (int i = 0; i < size; i++) {
                        if (i == rank)
                            continue;
                        packet_t *pkt = malloc(sizeof(packet_t));
                        pkt->data = room;
                        pkt->ts = ++lamport;
                        sendPacket(pkt, i, RELEASE);
                    }
                    
                    clearProcessQueue(&processQueue); // Wyczyszczenie kolejki procesów
                    // Nullowanie numeru zajętej sali
                    room = -1;

                    changeState( Start );
                }
                if (actorType == 1) {
                    changeState( Wait );
                }
                
            }
            // ZWOLNIENIE SALI
            if (stan == Release) {
                changes++;

                clearProcessQueue(&processQueue); // Wyczyszczenie kolejki procesów
                rooms[currentPacket.data]--;

                if (room != -1) {
                    println("Wychodzę z sali nr %d", room);
                    // Nullowanie numeru zajętej sali
                    room = -1;
                }

                changeState( Start );
            }
            // CZEKANIE
            if (stan == Wait) {
                changes++;
            }
        } else {
	        changeState( Finish );
        }
        sleep(SEC_IN_STATE);
    }
}
