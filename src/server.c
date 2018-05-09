/*
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2017/2018
    Semestar:       Zimski (V)

    Ime fajla:      server.c
    Opis:           TCP/IP server

    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/

#include "globalne_promenljive.h"
#include <stdio.h>     //printf
#include <string.h>    //memset
#include <arpa/inet.h>  //inet_aton
#include <sys/socket.h> //socket
#include <unistd.h>    //close
#include <math.h>

#define BUFLEN 512   // Max length of buffer
#define PORT 8888    // The port on which to listen for incoming data
#define EXCLAMATION '!' // the package is lost

unsigned int global; // in memoriam
unsigned int flag;   // in memoriam

int main(void)
{
    // Variable initialization
    //------------------------------------------
    struct sockaddr_in sock_server, sock_client;
    int s, slen = sizeof(sock_client), recv_len;
    char buf[BUFLEN];
    char previous[BUFLEN];
    int previous_len = 0;

    // Create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        puts("Could not create socket");
        return 1;
    }

    // Empty the structure
    memset((char*) &sock_server, 0, sizeof(sock_server));

    // socket initialization
    sock_server.sin_family = AF_INET;
    sock_server.sin_port = htons(PORT);
    sock_server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind
    if(bind(s, (struct sockaddr*)&sock_server, sizeof(sock_server)) == -1)
    {
        perror("Error: bind failed");
        return 1;
    }

    printf("\nWaiting for data...\n\n");
    fflush(stdout);

    int i = 0; // for better printing
    // Listen for data
    while(1)
    {
        printf("%d)\n\n", ++i);

        // Empty the string
        memset(buf, 0, BUFLEN);
        memset(previous, 0, BUFLEN);

        // First, we receive the value of the [global] for loop initialization
        if ((recv_len = recvfrom(s, (char*)buf, BUFLEN, 0, (struct sockaddr *) &sock_client, (socklen_t*)&slen)) == -1){
                        perror("recvfrom failed. Error");
                        return 1;
            }

        global = buf[0];

        // Send message back to client (send [global back])
        if (sendto(s, (char *)buf, recv_len, 0, (struct sockaddr*) &sock_client, slen) == -1) {
                        perror("sendto failed. Error");
                        return 1;
        }


        int k = 0; // initializing the loop variable
        do {

            // Empty the string
            memset(buf, 0, BUFLEN);



                // Receive data (blocking call)
                if ((recv_len = recvfrom(s, (char*)buf, BUFLEN, 0, (struct sockaddr *) &sock_client, (socklen_t*)&slen)) == -1)
                {
                            perror("recvfrom failed. Error");
                            return 1;
                }

                    // Print client info and received data
                    printf("Received packet from %s:%d\n", inet_ntoa(sock_client.sin_addr), ntohs(sock_client.sin_port));
                    printf("Data: %s\n" , buf);
                    printf("Length: %d\n\n" , recv_len);

                // put buffer information in previous, to save the package data
                if (buf[0] != EXCLAMATION) {

                // clear previous buffer and set the value
                memset(previous, 0, BUFLEN);
                    int m;
                        for (m = 0; m < recv_len; m++) {
                            previous[m] = buf[m];
                        }
                    previous_len = recv_len;

                }

            k++;   // next iteration

        } while (k < global);  // the server needs to receive [global] number of packages before sending back

                if (previous[0]) { // we have some package data in previous

                    if (sendto(s, (char *)previous, previous_len, 0, (struct sockaddr*) &sock_client, slen) == -1)
                    {
                        perror("sendto failed. Error");
                        return 1;
                    }
                }
                else { // we send the EXCLAMATION, which is in [buf]

                    // Send message back to client
                    if (sendto(s, (char *)buf, recv_len, 0, (struct sockaddr*) &sock_client, slen) == -1)
                    {
                        perror("sendto failed. Error");
                        return 1;
                    }
                }
        printf("\n\n"); // for better printing
    }

    close(s);  // closing socket
    return 0;
}
