#include <arpa/inet.h> /* address conversions */
#include <netdb.h> /* network database library */
#include <stdio.h> /* printf */
#include <string.h> /* memset, strlen */
#include <sys/socket.h> /* sockets */
#include <unistd.h> /* close */

#define NUM_ADDRESSES 3
#define BUFSIZE (10 * 1024) /* size of buffer, max 64 KByte */

static unsigned char buf[BUFSIZE]; /* receive buffer */
typedef struct {
    char address;
    char ip[INET_ADDRSTRLEN];
    int port;
} addressTable;

// Function to get the addressTable entry by address identifier
addressTable* getAddressEntry(addressTable array[], int size, char identifier)
{
    for (int i = 0; i < size; i++) {
        if (array[i].address == identifier) {
            return &array[i];
        }
    }
    return NULL;
}

void printAddressTable(addressTable array[], int size)
{
    printf("\nAddress Table:\n");
    printf("Address  | IP Address    | Port\n");
    printf("---------------------------------\n");
    for (int i = 0; i < size; i++) {
        if (array[i].address != '\0') { // Print only non-empty entries
            printf("    %c    | %s | %d\n", array[i].address, array[i].ip, array[i].port);
        }
    }
    printf("---------------------------------\n");
}

int main(void)
{
    addressTable array[NUM_ADDRESSES];

    // Initialize the address field of each entry in the table
    for (int i = 0; i < NUM_ADDRESSES; i++) {
        array[i].address = '\0'; // Initialize address to indicate an empty entry
        strcpy(array[i].ip, ""); // Initialize IP to an empty string
        array[i].port = 0; // Initialize port to 0
    }

    struct sockaddr_in myaddr; /* our address */
    struct sockaddr_in remaddr; /* remote address */
    struct sockaddr_in forwardaddr; /* address to forward the message to */
    socklen_t addrlen = sizeof(remaddr); /* length of addresses */
    int recvlen; /* # bytes received */
    int fd; /* socket file descriptor */
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
        close(fd);
        return 0;
    }

    /* now loop, receiving data and printing what we received */
    for (;;) {
        printf("waiting on port %d\n", ntohs(myaddr.sin_port));
        recvlen = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&remaddr, &addrlen);
        if (recvlen > 0) {
            buf[recvlen] = '\0';
            printf("received message: \"%s\" (%d bytes)\n", buf, recvlen);

            inet_ntop(AF_INET, &(remaddr.sin_addr), client_ip, INET_ADDRSTRLEN);
            int client_port = ntohs(remaddr.sin_port);

            printf("Client IP: %s, Port: %d\n", client_ip, client_port);

            // The first character of the buffer is the sender identifier
            char sender_identifier = buf[0];
            // The second character of the buffer is the destination identifier
            char destination_identifier = buf[1];

            printAddressTable(array, NUM_ADDRESSES);

            // Update or add the sender information
            addressTable* sender_entry = getAddressEntry(array, NUM_ADDRESSES, sender_identifier);
            if (sender_entry == NULL) {
                // Find an empty slot to add the new sender entry
                for (int i = 0; i < NUM_ADDRESSES; i++) {
                    if (array[i].address == '\0') {
                        array[i].address = sender_identifier;
                        strcpy(array[i].ip, client_ip);
                        array[i].port = client_port;
                        printf("New sender entry added: %c, IP = %s, Port = %d\n", array[i].address, array[i].ip, array[i].port);
                        break;
                    }
                }
            } else {
                // Update existing sender entry
                strcpy(sender_entry->ip, client_ip);
                sender_entry->port = client_port;
                printf("Updated sender entry: %c, IP = %s, Port = %d\n", sender_entry->address, sender_entry->ip, sender_entry->port);
            }

            printAddressTable(array, NUM_ADDRESSES);

            // Find the destination entry and forward the message if it exists
            addressTable* destination_entry = getAddressEntry(array, NUM_ADDRESSES, destination_identifier);
            if (destination_entry != NULL) {
                // Set up the forwarding address
                memset((char*)&forwardaddr, 0, sizeof(forwardaddr));
                forwardaddr.sin_family = AF_INET;
                forwardaddr.sin_port = htons(destination_entry->port);

                if (inet_pton(AF_INET, destination_entry->ip, &forwardaddr.sin_addr) <= 0) {
                    fprintf(stderr, "Invalid IP address format: %s\n", destination_entry->ip);
                    continue;
                }

                // Forward the message to the new IP address
                printf("Forwarding message to %s:%d\n", destination_entry->ip, destination_entry->port);
                if (sendto(fd, buf + 2, recvlen - 2, 0, (struct sockaddr*)&forwardaddr, sizeof(forwardaddr)) < 0) {
                    perror("sendto");
                }
            } else {
                printf("Destination identifier '%c' not found in the address table.\n", destination_identifier);
            }
        } else {
            printf("uh oh - something went wrong!\n");
        }
    }
    close(fd);
    return 0;
}
