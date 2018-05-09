/*
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2017/2018
    Semestar:       Zimski (V)

    Ime fajla:      client.c
    Opis:           UDP/IP klijent

    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/

#include "globalne_promenljive.h"

#include<stdio.h>      //printf
#include <stdlib.h>	   //random
#include<string.h>     //memset
#include<arpa/inet.h>  //inet_aton
#include<sys/socket.h> //socket
#include <unistd.h>    //close
#include<math.h>
#include <errno.h>     //exit

#define SERVER "127.0.0.1" // we use the local address
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to send data
#define MOD 5 //we make probability partial with number MOD
#define LOSS 20 // LOSS = 100 / MOD
#define EXCLAMATION '!'  // information of package loss
#define LOST 0
#define SENT 1

unsigned int global; // in memoriam
unsigned int flag; // signal to server, if he is receiving a package or not

int probability(int);   // function that calculates the probability
int should_receive(int);  // whether or not the package is lost (1 -> lost, 0 -> )

int main(int argNum, char* arg[])
{
    //checking for errors during compiling
    if (argNum != 4) {
        printf("Error: Number of arguments does not match:\n");
        printf("arg[0] -> ./client\n");
        printf("arg[1] -> window_size\n");
        printf("arg[2] -> number_of_packages\n");
        printf("arg[3] -> probability_loss_percentage\n");
        exit(-1);
    }

    if (atoi(arg[1]) < 0 || atoi(arg[2]) < 0) {
        printf("Error: [window_size] and [number_of_packages] must be greater than 0\n");
        exit(-1);
    }

    if (atoi(arg[2]) <= atoi(arg[1])) {
        printf("Error: [window_size] must be less than [number_of_packages]\n");
        exit(-1);
    }

    if (atoi(arg[3]) < 0 || atoi(arg[3]) > 100) {
        printf("Error: [probability_loss_percentage] must be between 0 and 100\n");
        exit(-1);
    }


    //Initialization of packages
    char paketi[atoi(arg[2])][atoi(arg[2])];
    printf("Packages: \n");

    int i;
        for (i = 0; i < atoi(arg[2]); i++) {
            int duzina = i + 1;
            int j;
            printf("Package %d: ", i + 1);
                for (j = 0; j < duzina; j++) {
                    paketi[i][j] = rand() % 26 + 97;
                    printf("%c", paketi[i][j]);
                }
            paketi[i][duzina] = '\0';
            printf("\n");

        }


    //Creating UDP socket
    struct sockaddr_in sock_server;
    int s, slen = sizeof(sock_server);
    char buf[BUFLEN];


    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        puts("Could not create socket");
        return 1;
    }

    memset((char *) &sock_server, 0, sizeof(sock_server));
    sock_server.sin_family = AF_INET;
    sock_server.sin_port = htons(PORT);


    // Check if address is valid
    if (inet_aton(SERVER, &sock_server.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton failed. Error\n");
        return 1;
    }


    // Variable initialization
    //--------------------------------------------------------------------------
    global = atoi(arg[1]); // initializing global variable (global = window_size)
    int losing = probability(atoi(arg[3])); // setting variable for probability
    int br = 0; // number of window sliding
    int brPaketa = 0; //number of received packages
    int pomeranja = atoi(arg[2]) - atoi(arg[1]); // number of required window sliding
    int primljeni[atoi(arg[2])]; // array of received packages
    char c = EXCLAMATION; // tell server to stop receiving for the current iteration



    // Initializing the array of received packages (initial all values are 0
   // --------------------------------------------------------------------------
        int f;
        for (f = 0; f < sizeof(primljeni) / 4; f++) {
            primljeni[f] = 0;
        }

    printf("\n\n");
    int w = 0; // for better printing

    // Sending packages (MAIN WHILE)
    //-----------------------------------------------------------------------------------------------------------
    while(1) {

        printf("%d)\n\n", ++w);

        // First, we send global variable, so the server knows how many packages he needs to receive
        if (sendto(s, &global, sizeof(global) / 4, 0, (struct sockaddr *) &sock_server, (socklen_t)slen)==-1) {
                    perror("Slanje nije uspelo!");
                    return 1;
        }

        // Clear old buffer, and wait for server response
        memset(buf, 0, BUFLEN);
        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &sock_server, (socklen_t*)&slen) == -1) {
                    perror("recvfrom failed. Error");
                    return 1;
        }

        int k;
        printf("GLOBAL = %d\n", global);
            for (k = 0 + br; k < atoi(arg[1]) + br; k++) {  //window that sends the packages, shifted [br] times

                flag = should_receive(losing);
                printf("FLAG = %d\n", flag);

                    if (flag) {
                        if (primljeni[k] == 0) { //if package number k is not sent, send it again
                            if (sendto(s, paketi[k], strlen(paketi[k]), 0, (struct sockaddr *) &sock_server, (socklen_t)slen)==-1) {
                                    perror("Error: sending failed!");
                                    return 1;
                            }
                        }
                    }
                    else {
                        if (primljeni[k] == 0) { //if package number k is not sent, send [c] -> EXCLAMATION
                            if (sendto(s, &c, sizeof(c), 0, (struct sockaddr *) &sock_server, (socklen_t)slen)==-1) {
                                        perror("Error: sending failed!");
                                        return 1;
                            }
                        }
                    }

            }


            // Now, we wait for server to inform us that he has received something
            //----------------------------------------------------------------------------------------------------------------
                        // Clear old buffer
                        memset(buf, 0, BUFLEN);
                        // We put the length of the received information in the variable [pismo]
                        int pismo = 0;

                        if ((pismo = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &sock_server, (socklen_t*)&slen)) == -1) {
                            perror("Error: receive failed!");
                            return 1;
                        }

                        printf("BUFFER = %s\n", buf);
                        printf("LENGTH = %d\n", pismo);

                        // this will occur only if we all the packages have been lost
                        if (buf[0] != c) {

                            // comparing package_length
                            for (k = 0; k < atoi(arg[2]); k++) {
                                if (strlen(paketi[k]) == pismo) {
                                    break;
                                }
                            }

                            if (primljeni[k] == 0) {   // if the package is not received yet
                                primljeni[k] = 1;      // put 1 in the spot for the package
                                brPaketa += 1;         // number of packages increase by 1
                                global--;              // decrease the number of packages that the server must receive
                            }
                        }

            printf("Packages_number = %d\n", brPaketa);

            if (primljeni[0 + br] == 1 && br != pomeranja) { // if the first package of the window is received and it is not the last position
                br++;
                global++;
                printf("SHIFTS = %d\n", br);
            }
            printf("\n");

            // criteria for breaking for the while(1) loop
            if (brPaketa == atoi(arg[2]) && br == pomeranja) { break; } // all the packages have been received and it is the last position
    }

    close(s);  // closing the socket
    return 0;
}

int probability(int percentage) {

    int local;
    int ratio = MOD / 2;

        if ((percentage % MOD) <= ratio) {
            local = percentage - (percentage % MOD);
        }
        else {
            local = percentage - (percentage % MOD) + MOD;
        }

    return local;

}

// 0 - don't receive because of package loss, 1 - receive
int should_receive(int percentage) {

    int numbers[percentage / MOD];
    int value;

    int i;
        for (i = 0; i < percentage / MOD; i++) {
            numbers[i] = i;
        }

    int rand_number = rand() % LOSS;

        for (i = 0; i < percentage / MOD; i++) {
            if (numbers[i] == rand_number) {
                value = LOST;
                return value;
            }
        }
    value = SENT;
    return value;

}
