/*
 * calc_tcp_server.c - Connection-Oriented (TCP) Iterative Calculator Server
 *
 * This server listens for incoming TCP connections from clients.
 * For each client, it iteratively receives calculation requests,
 * performs the calculation using calc_logic.c, and sends back the result.
 * It handles one client completely before accepting the next.
 *
 * Compile: gcc -std=c99 -Wall -o calc_tcp_server calc_tcp_server.c calc_logic.c
 * Run: ./calc_tcp_server [port]
 */

#include "calc_common.h" // Common definitions (OperationType, CalculatorRequest, CalculatorResponse)
#include <stdio.h>       // For printf, fprintf, perror
#include <stdlib.h>      // For EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>      // For memset
#include <unistd.h>      // For close
#include <errno.h>       // For errno
#include <sys/types.h>   // For socket, bind, listen, accept
#include <sys/socket.h>  // For socket, bind, listen, accept
#include <netinet/in.h>  // For sockaddr_in, INADDR_ANY
#include <arpa/inet.h>   // For inet_ntop (to get client IP address)

#define DEFAULT_PORT 6000    // Default port number for the server
#define BACKLOG      5       // Number of pending connections queue will hold
#define BUFFER_SIZE  sizeof(CalculatorRequest) // Buffer size for requests/responses

// Function to handle a single client's requests iteratively
void handle_client(int client_socket);

int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int port = DEFAULT_PORT;

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

    // 1. Create socket (TCP)
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("ERROR: Could not create socket");
        return EXIT_FAILURE;
    }
    printf("Server socket created successfully.\n");

    // Optional: Set socket option to reuse address (prevents "Address already in use" error)
    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("WARNING: setsockopt(SO_REUSEADDR) failed");
    }

    // 2. Prepare the sockaddr_in structure
    memset(&server_addr, 0, sizeof(server_addr)); // Clear the structure
    server_addr.sin_family = AF_INET;             // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;     // Listen on all available network interfaces
    server_addr.sin_port = htons(port);           // Port in network byte order

    // 3. Bind socket to the specified IP and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Could not bind socket");
        close(server_socket);
        return EXIT_FAILURE;
    }
    printf("Server socket bound to port %d.\n", port);

    // 4. Start listening for incoming connections
    if (listen(server_socket, BACKLOG) < 0) {
        perror("ERROR: Could not listen on socket");
        close(server_socket);
        return EXIT_FAILURE;
    }
    printf("TCP Calculator Server ready, listening on port %d...\n", port);

    while (1) { // Main server loop: accept and handle clients iteratively
        printf("\nWaiting for a new client connection...\n");

        // 5. Accept a new client connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            // Check if error is due to interrupted system call
            if (errno == EINTR) {
                continue; // Retry accept
            }
            perror("ERROR: Failed to accept connection");
            continue; // Continue waiting for other connections
        }

        // Get client details for logging/display
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Connection accepted from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        // 6. Handle client requests iteratively
        handle_client(client_socket);

        // 7. Close the client socket after handling all its requests
        printf("Client %s:%d disconnected. Closing client socket.\n", client_ip, ntohs(client_addr.sin_port));
        close(client_socket);
    }

    // This part is typically unreachable in a server that runs indefinitely
    close(server_socket);
    return EXIT_SUCCESS;
}

// --- handle_client Function Implementation ---
void handle_client(int client_socket) {
    CalculatorRequest request;
    CalculatorResponse response;
    ssize_t bytes_received;

    while (1) { // Loop to handle multiple requests from the same client
        // Clear the request structure before receiving
        memset(&request, 0, sizeof(CalculatorRequest));

        // 1. Receive data (CalculatorRequest) from the client
        bytes_received = recv(client_socket, &request, sizeof(CalculatorRequest), 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected gracefully.\n");
            } else {
                perror("ERROR: recv failed");
            }
            break; // Exit inner loop if client disconnects or error occurs
        }

        // Validate received size (important for binary protocols)
        if (bytes_received != sizeof(CalculatorRequest)) {
            fprintf(stderr, "WARNING: Received incomplete request (expected %lu bytes, got %zd).\n",
                    sizeof(CalculatorRequest), bytes_received);
            // Optionally, send an error response back to client here
            response.status = -1; // General error
            response.result = 0.0;
            send(client_socket, &response, sizeof(CalculatorResponse), 0);
            continue; // Skip to next request from this client
        }

        printf("Received request: Operation %d, Num1=%.2lf, Num2=%.2lf\n",
               request.operation, request.num1, request.num2);

        // 2. Process the request (perform calculation)
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

        // 3. Send data (CalculatorResponse) back to the client
        if (send(client_socket, &response, sizeof(CalculatorResponse), 0) < 0) {
            perror("ERROR: send failed");
            break; // Exit inner loop if send fails
        }
        printf("Sent response: Status=%d, Result=%.2lf\n", response.status, response.result);
    }
}