/*
 * calc_udp_server.c - Connectionless (UDP) Iterative Calculator Server
 *
 * This server uses UDP to receive calculation requests as datagrams.
 * For each received request, it performs the calculation using calc_logic.c
 * and sends the result back as a datagram to the client that sent the request.
 *
 * Compile: gcc -std=c99 -Wall -o calc_udp_server calc_udp_server.c calc_logic.c
 * Run: ./calc_udp_server [port]
 */

#include "calc_common.h" // Common definitions (OperationType, CalculatorRequest, CalculatorResponse)
#include <stdio.h>       // For printf, fprintf, perror
#include <stdlib.h>      // For EXIT_SUCCESS, EXIT_FAILURE, atoi
#include <string.h>      // For memset
#include <unistd.h>      // For close
#include <sys/types.h>   // For socket, bind
#include <sys/socket.h>  // For socket, bind, recvfrom, sendto
#include <netinet/in.h>  // For sockaddr_in, INADDR_ANY
#include <arpa/inet.h>   // For inet_ntop

#define DEFAULT_PORT 6001    // Default port number for the UDP server
#define BUFFER_SIZE  sizeof(CalculatorRequest) // Buffer size for requests/responses

int main(int argc, char *argv[]) {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int port = DEFAULT_PORT;
    CalculatorRequest request;
    CalculatorResponse response;
    ssize_t bytes_received;

    // Parse command line arguments for port number
    if (argc == 2) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number. Using default port %d.\n", DEFAULT_PORT);
            port = DEFAULT_PORT;
        }
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // 1. Create UDP socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("ERROR: Could not create UDP socket");
        return EXIT_FAILURE;
    }
    printf("UDP server socket created successfully.\n");

    // 2. Prepare the sockaddr_in structure
    memset(&server_addr, 0, sizeof(server_addr)); // Clear the structure
    server_addr.sin_family = AF_INET;             // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;     // Listen on all available network interfaces
    server_addr.sin_port = htons(port);           // Port in network byte order

    // 3. Bind socket to the specified IP and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Could not bind UDP socket");
        close(server_socket);
        return EXIT_FAILURE;
    }
    printf("UDP Calculator Server bound to port %d. Waiting for requests...\n", port);

    while (1) { // Main server loop: receive and respond to datagrams
        // Clear the request structure before receiving
        memset(&request, 0, sizeof(CalculatorRequest));

        // 4. Receive data (CalculatorRequest) from any client
        // recvfrom also fills in the client's address (client_addr)
        bytes_received = recvfrom(server_socket, &request, sizeof(CalculatorRequest), 0,
                                  (struct sockaddr *)&client_addr, &client_len);

        if (bytes_received < 0) {
            perror("ERROR: recvfrom failed");
            continue; // Continue to wait for next datagram
        }

        // Validate received size (important for binary protocols)
        if (bytes_received != sizeof(CalculatorRequest)) {
            fprintf(stderr, "WARNING: Received incomplete request (expected %lu bytes, got %zd).\n",
                    sizeof(CalculatorRequest), bytes_received);
            // In UDP, errors usually mean dropping the packet or sending a specific error datagram.
            // For now, we'll just log and continue.
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("\nReceived request from %s:%d: Operation %d, Num1=%.2lf, Num2=%.2lf\n",
               client_ip, ntohs(client_addr.sin_port), request.operation, request.num1, request.num2);

        // 5. Process the request (perform calculation)
        response.status = 0; // Assume success
        response.result = 0.0; // Default result

        switch (request.operation) {
            case ADD:
                response.result = add(request.num1, request.num2);
                break;
            case SUBTRACT:
                response.result = subtract(request.num1, request.num2);
                break;
            case MULTIPLY:
                response.result = multiply(request.num1, request.num2);
                break;
            case DIVIDE:
                if (request.num2 == 0.0) {
                    response.status = -1; // Error: Division by zero
                    fprintf(stderr, "Error: Division by zero requested.\n");
                } else {
                    response.result = divide(request.num1, request.num2);
                }
                break;
            default:
                response.status = -1; // Error: Invalid operation
                fprintf(stderr, "Error: Invalid operation received (%d).\n", request.operation);
                break;
        }

        // 6. Send data (CalculatorResponse) back to the client that sent the request
        if (sendto(server_socket, &response, sizeof(CalculatorResponse), 0,
                   (struct sockaddr *)&client_addr, client_len) < 0) {
            perror("ERROR: sendto failed");
            // In UDP, if sendto fails, the client won't get a response. We just log and continue.
        }
        printf("Sent response to %s:%d: Status=%d, Result=%.2lf\n",
               client_ip, ntohs(client_addr.sin_port), response.status, response.result);
    }

    // This part is typically unreachable in a server that runs indefinitely
    close(server_socket);
    return EXIT_SUCCESS;
}