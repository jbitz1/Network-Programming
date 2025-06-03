/*
 * calc_tcp_client.c - Connection-Oriented (TCP) Iterative Calculator Client
 *
 * This client connects to a TCP calculator server, allowing the user
 * to perform arithmetic operations by sending requests to the server
 * and receiving responses.
 *
 * Compile: gcc -std=c99 -Wall -o calc_tcp_client calc_tcp_client.c
 * Run: ./calc_tcp_client [server_ip] [port]
 */

#include "calc_common.h" // Common definitions (OperationType, CalculatorRequest, CalculatorResponse)
#include <stdio.h>       // For printf, fprintf, perror
#include <stdlib.h>      // For EXIT_SUCCESS, EXIT_FAILURE, atoi
#include <string.h>      // For memset, strcmp
#include <unistd.h>      // For close
#include <arpa/inet.h>   // For inet_addr, htons
#include <sys/socket.h>  // For socket, connect, send, recv
#include <netinet/in.h>  // For sockaddr_in

#define DEFAULT_SERVER_IP "127.0.0.1" // Default server IP address (localhost)
#define DEFAULT_PORT      6000        // Default server port number

// Function to display the calculator menu
void display_menu();

int main(int argc, char *argv[]) {
    int client_socket;
    struct sockaddr_in server_addr;
    char *server_ip = DEFAULT_SERVER_IP;
    int port = DEFAULT_PORT;
    int choice;
    double num1, num2;
    CalculatorRequest request;
    CalculatorResponse response;
    ssize_t bytes_sent, bytes_received;

    // Parse command line arguments for server IP and port
    if (argc == 3) {
        server_ip = argv[1];
        port = atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number. Using default port %d.\n", DEFAULT_PORT);
            port = DEFAULT_PORT;
        }
    } else if (argc == 2) {
        server_ip = argv[1]; // Only IP provided, use default port
    } else if (argc > 3) {
        fprintf(stderr, "Usage: %s [server_ip] [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // 1. Create socket (TCP)
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("ERROR: Could not create client socket");
        return EXIT_FAILURE;
    }
    printf("Client socket created.\n");

    // 2. Prepare the sockaddr_in structure for the server
    memset(&server_addr, 0, sizeof(server_addr)); // Clear the structure
    server_addr.sin_family = AF_INET;             // IPv4
    server_addr.sin_addr.s_addr = inet_addr(server_ip); // Server IP address
    server_addr.sin_port = htons(port);           // Server port in network byte order

    // 3. Connect to the server
    printf("Attempting to connect to server at %s:%d...\n", server_ip, port);
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Failed to connect to server");
        close(client_socket);
        return EXIT_FAILURE;
    }
    printf("Successfully connected to the calculator server.\n");

    while (1) { // Loop for client interaction
        display_menu(); // Show the menu options
        printf("Enter your choice: ");

        // Read user's choice
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            // Clear input buffer
            while (getchar() != '\n');
            continue; // Go back to the beginning of the loop
        }

        if (choice == 0) { // Exit option
            printf("Exiting client. Goodbye!\n");
            break; // Exit the loop
        }

        // Validate choice and get numbers if it's an operation
        if (choice >= ADD && choice <= DIVIDE) {
            printf("Enter first number: ");
            if (scanf("%lf", &num1) != 1) {
                printf("Invalid input. Please enter a valid number.\n");
                while (getchar() != '\n');
                continue;
            }

            printf("Enter second number: ");
            if (scanf("%lf", &num2) != 1) {
                printf("Invalid input. Please enter a valid number.\n");
                while (getchar() != '\n');
                continue;
            }

            // Populate the request structure
            request.operation = (OperationType)choice;
            request.num1 = num1;
            request.num2 = num2;

            // 4. Send the request to the server
            bytes_sent = send(client_socket, &request, sizeof(CalculatorRequest), 0);
            if (bytes_sent < 0) {
                perror("ERROR: send failed");
                break; // Exit loop on send error
            }
            if (bytes_sent != sizeof(CalculatorRequest)) {
                fprintf(stderr, "WARNING: Sent incomplete request (expected %lu, sent %zd).\n",
                        sizeof(CalculatorRequest), bytes_sent);
            }
            printf("Request sent to server.\n");

            // 5. Receive the response from the server
            bytes_received = recv(client_socket, &response, sizeof(CalculatorResponse), 0);
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    printf("Server closed the connection unexpectedly.\n");
                } else {
                    perror("ERROR: recv failed");
                }
                break; // Exit loop on receive error or server disconnect
            }

            // Validate received response size
            if (bytes_received != sizeof(CalculatorResponse)) {
                fprintf(stderr, "WARNING: Received incomplete response (expected %lu bytes, got %zd).\n",
                        sizeof(CalculatorResponse), bytes_received);
                printf("Server response malformed.\n");
            } else {
                // 6. Display the result or error
                if (response.status == 0) {
                    printf("Server Result: %.2lf\n", response.result);
                } else {
                    printf("Server Error: ");
                    if (request.operation == DIVIDE && request.num2 == 0) {
                        printf("Division by zero.\n");
                    } else {
                        printf("Operation failed or invalid input on server.\n");
                    }
                }
            }
        } else {
            printf("Invalid choice. Please enter a number between 0 and 4.\n");
        }
        printf("\n"); // Add a newline for better readability
    }

    // 7. Close the client socket
    printf("Disconnecting from server.\n");
    close(client_socket);
    return EXIT_SUCCESS;
}

// --- display_menu Function Implementation ---
void display_menu() {
    printf("-------------------------\n");
    printf("  Client Calculator Menu\n");
    printf("-------------------------\n");
    printf("1. Add\n");
    printf("2. Subtract\n");
    printf("3. Multiply\n");
    printf("4. Divide\n");
    printf("0. Exit\n");
    printf("-------------------------\n");
}