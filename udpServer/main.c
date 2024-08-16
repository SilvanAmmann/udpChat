#include <arpa/inet.h> /* address conversions */
#include <netdb.h> /* network database library */
#include <stdio.h> /* printf */
#include <string.h> /* memset, strlen */
#include <sys/socket.h> /* sockets */
#include <unistd.h>
#define NUM_ADDRESSES 3
#define BUFSIZE (10 * 1024) /* size of buffer, max 64 KByte */

static unsigned char buf[BUFSIZE]; /* receive buffer */
typedef struct {
    char address;
    char string[15];
} addressTable;
// Function to get the IP address for a given character identifier
const char* getIPAddress(addressTable array[], int size, char identifier)
{
    for (int i = 0; i < size; i++) {
        if (array[i].address == identifier) {
            return array[i].string;
        }
    }
    return NULL; // Return NULL if the identifier is not found
}

int main(void)
{
    addressTable array[NUM_ADDRESSES] = {
        { '0', "146.136.90.44" },
        { '1', "146.136.90.45" },
        { '2', "146.136.90.46" },
    };

    struct sockaddr_in myaddr; /* our address */
    struct sockaddr_in remaddr; /* remote address */
    struct sockaddr_in forwardaddr; /* address to forward the message to */
    socklen_t addrlen = sizeof(remaddr); /* length of addresses */
    int recvlen; /* # bytes received */
    int fd; /* socket file descriptor */
    int msgcnt = 0; /* count # of messages we received */
    const int port = 1234; /* port used for socket */
    char client_ip[INET_ADDRSTRLEN]; /* buffer for client's IP address */
    /* create a UDP socket */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket\n");
        return 0;
    }

    /* bind the socket to any valid IP address and a specific port */
    memset((char*)&myaddr, 0, sizeof(myaddr)); /* initialize all fields */
    myaddr.sin_family = AF_INET; /* Internet protocol */
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY); /* accept all incoming */
    myaddr.sin_port = htons(port); /* port number for socket */

    if (bind(fd, (struct sockaddr*)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        return 0;
    }

    /* now loop, receiving data and printing what we received */
    for (;;) {
        printf("waiting on port %d\n", port);
        recvlen = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&remaddr, &addrlen);
        if (recvlen > 0) {
            buf[recvlen] = '\0'; /* terminate buffer */
            printf("received message: \"%s\" (%d bytes)\n", buf, recvlen);
            // Convert the client's address to a human-readable format
            inet_ntop(AF_INET, &(remaddr.sin_addr), client_ip, INET_ADDRSTRLEN);
            printf("Client IP: %s, Port: %d\n", client_ip, ntohs(remaddr.sin_port));
            // Extract the address and the message
            char identifier = buf[0];
            char* message = (char*)&buf[0];

            // Get the forwarding IP address
            const char* ipAddress = getIPAddress(array, NUM_ADDRESSES, identifier);
            if (ipAddress != NULL) {
                // Set up the forwarding address
                memset((char*)&forwardaddr, 0, sizeof(forwardaddr));
                forwardaddr.sin_family = AF_INET;
                forwardaddr.sin_port = htons(51417); /* port number to forward to */

                if (inet_pton(AF_INET, ipAddress, &forwardaddr.sin_addr) <= 0) {
                    fprintf(stderr, "Invalid IP address format: %s\n", ipAddress);
                    continue;
                }

                // Forward the message to the new IP address
                printf("Forwarding message to %s: \"%s\"\n", ipAddress, message);
                if (sendto(fd, message, strlen(message), 0, (struct sockaddr*)&forwardaddr, sizeof(forwardaddr)) < 0) {
                    perror("sendto");
                }
            } else {
                printf("Identifier '%c' not found in the address table.\n", identifier);
            }
        } else {
            printf("uh oh - something went wrong!\n");
        }
    }
    close(fd);
    return 0;
}
