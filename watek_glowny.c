#include "main.h"
#include "watek_glowny.h"

#define TANCERKA 0
#define GITARZYSTA 1
#define NUM_OF_TANCERKI 3
#define ROOMS 4

void mainLoop()
{
    srandom(rank);

    int rooms[ROOMS] = {0, 0, 0, 0}; // sale
    int room = -1; // sala, w której się znajdujemy
    int synAcks = 0; // liczba odebanych ACK. Do sprawdzania czy kolejka jest stworzona

    Packet currentPacket; // obecnie odczytywany pakiet

    // obliczanie typu postaci
    int actorType = 1;
    if (rank < NUM_OF_TANCERKI)
        actorType = 0;


    if (actorType == TANCERKA) {
	    println("Tancerka gotowa");
    } else {
	    println("Gitarzysta gotowy");
    }

    while (stan != Finish) {
        if (lamport<MAX_LAMPORT) {

            // POBIERANIE DANYCH PAKIETU PIERWSZEGO W KOLEJCE
            if (stan != Start && packetQueue != NULL) {
                currentPacket = getFirstPacket(&packetQueue);

                // Zmiana stanu na podstawie pakietu
                switch(currentPacket.tag) {
                    case SYN:
                        changeState( SendSynAck );
                    break;
                    case SYNACK:
                        changeState( CheckQueue );
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
            }

            // POCZĄTEK
            if (stan == Start) {

                debug("{%d, %d, %d, %d}", rooms[0], rooms[1], rooms[2], rooms[3]);

                println("Wysyłam swoje dane do wszystkich");

                // Wysyłanie SYN do wszytkich
                lamport++;
                packet_t *pkt = malloc(sizeof(packet_t));
                pkt->data = actorType;
                pkt->ts = lamport;
                pthread_mutex_lock( &stateMut );
                for (int i = 0; i < size; i++) {
                    if (i == rank)
                        continue;
                    sendPacket(pkt, i, SYN);
                }

                // Dodaj siebie do kolejki
                putProcess(&processQueue, rank, pkt->ts, actorType);
                pthread_mutex_unlock( &stateMut );

                changeState( Wait );
	        }

            // DODANIE PROCESU NADAWCY DO KOLEJKI
            // ODESYŁANIE SYNACK
            if (stan == SendSynAck) {

                putProcess(&processQueue, currentPacket.src, currentPacket.ts, currentPacket.data);
                debug("Dodałem proces %d do kolejki", currentPacket.src);

                packet_t *pkt = malloc(sizeof(packet_t));
                pkt->data = actorType;
                pkt->ts = ++lamport;
                sendPacket(pkt, currentPacket.src, SYNACK);

                changeState( Wait );
            } 

            // SPRAWDZANIE CZY KOLEJKA JEST STWORZONA
            if (stan == CheckQueue) {

                synAcks++;

                if (synAcks >= size - 1) { // Tworzenie kolejki zakończone
                    println("Kolejka stworzona");

                    // Wyświetlanie lokalnej kolejki (debug)
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

                rooms[currentPacket.data] = 1;

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
                    int r = 0;
                    do {
                        if (r > 10) // zabezpieczenie przed nieskończoną pętlą
                            break;
                        room = random()%ROOMS;
                        r++;
                    }
                    while(rooms[room] != 0);

                    if (rooms[room] == 0) {
                        rooms[room] = 1;
                        // Wysyłanie REQ_ROOM do tancerek
                        println("Wysyłam żądania o %d salę", room);
                        lamport++;
                        packet_t *pkt = malloc(sizeof(packet_t));
                        pkt->data = room;
                        pkt->ts = lamport;
                        Process *p = processQueue;
                        while(p != NULL) {
                            if (p->type == 0 && p->id != rank) {
                                sendPacket(pkt, p->id, REQ_ROOM);
                            }
                            p = p->next;
                        }

                        // Wysyłanie IM_FIRST do tancerek
                        println("Wysyłam info, że jestem pierwsza do gitarzystów");
                        lamport++;
                        pkt = malloc(sizeof(packet_t));
                        pkt->data = room;
                        pkt->ts = lamport;
                        p = processQueue;
                        while(p != NULL) {
                            if (p->type == 1) {
                                sendPacket(pkt, p->id, IM_FIRST);
                            }
                            p = p->next;
                        }
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

                // Przybywaj w sali przez losowy czas
                sleep(random()%3+1);
                
                if (actorType == 0) {
                    println("Wychodzę z sali nr %d", room);

                    // Wysyłanie RELEASE do wszytkich
                    lamport++;
                    packet_t *pkt = malloc(sizeof(packet_t));
                    pkt->data = room;
                    pkt->ts = lamport;
                    for (int i = 0; i < size; i++) {
                        if (i == rank)
                            continue;
                        sendPacket(pkt, i, RELEASE);
                    }
                    
                    clearProcessQueue(&processQueue); // Wyczyszczenie kolejki procesów
                    rooms[room] = 0; // Czyszczenie zajętości sali
                    synAcks = 0; // Zerowanie ilości potwierdzeń
                    room = -1; // Nullowanie numeru zajętej sali

                    changeState( Start );
                }
                if (actorType == 1) {
                    changeState( Wait );
                }
                
            }
            // ZWOLNIENIE SALI
            if (stan == Release) {

                clearProcessQueue(&processQueue); // Wyczyszczenie kolejki procesów
                rooms[currentPacket.data] = 0; // Czyszczenie zajętości sali
                synAcks = 0; // Zerowanie ilości potwierdzeń

                if (room != -1) {
                    println("Wychodzę z sali nr %d", room);
                    // Nullowanie numeru zajętej sali
                    room = -1;
                }

                changeState( Start );
            }
            // CZEKANIE
            if (stan == Wait) {
                debug("Czekam");
            }
        } else {
	        changeState( Finish );
        }
        
        sleep(SEC_IN_STATE);
    }
}
