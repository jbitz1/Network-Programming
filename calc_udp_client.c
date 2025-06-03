/*
 * calc_udp_client.c - Connectionless (UDP) Iterative Calculator Client
 *
 * This client sends calculation requests as datagrams to a UDP calculator server
 * and receives responses as datagrams.
 *
 * Compile: gcc -std=c99 -Wall -o calc_udp_client calc_udp_client.c
 * Run: ./calc_udp_client [server_ip] [port]
 */

#include "calc_common.h" // Common definitions (OperationType, CalculatorRequest, CalculatorResponse)
#include <stdio.h>       // For printf, fprintf, perror
#include <stdlib.h>      // For EXIT_SUCCESS, EXIT_FAILURE, atoi
#include <string.h>      // For memset, strcmp
#include <unistd.h>      // For close
#include <arpa/inet.h>   // For inet_addr, htons
#include <sys/socket.h>  // For socket, sendto, recvfrom
#include <netinet/in.h>  // For sockaddr_in

#define DEFAULT_SERVER_IP "127.0.0.1" // Default server IP address (localhost)
#define DEFAULT_PORT      6001        // Default server port number for UDP

// Function to display the calculator menu
void display_menu();

int main(int argc, char *argv[]) {
    int client_socket;
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr); // Needed for sendto/recvfrom
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

    // 1. Create UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("ERROR: Could not create UDP client socket");
        return EXIT_FAILURE;
    }
    printf("UDP client socket created.\n");

    // 2. Prepare the sockaddr_in structure for the server
    memset(&server_addr, 0, sizeof(server_addr)); // Clear the structure
    server_addr.sin_family = AF_INET;             // IPv4
    server_addr.sin_addr.s_addr = inet_addr(server_ip); // Server IP address
    server_addr.sin_port = htons(port);           // Server port in network byte order

    printf("UDP Calculator Client ready. Sending requests to %s:%d\n", server_ip, port);

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

            // 3. Send the request (datagram) to the server
            bytes_sent = sendto(client_socket, &request, sizeof(CalculatorRequest), 0,
                                (struct sockaddr *)&server_addr, server_len);
            if (bytes_sent < 0) {
                perror("ERROR: sendto failed");
                break; // Exit loop on send error
            }
            if (bytes_sent != sizeof(CalculatorRequest)) {
                fprintf(stderr, "WARNING: Sent incomplete request (expected %lu, sent %zd).\n",
                        sizeof(CalculatorRequest), bytes_sent);
            }
            printf("Request sent to server.\n");

            // 4. Receive the response (datagram) from the server
            // For UDP, we use server_len again as the expected size of the sender's address
            bytes_received = recvfrom(client_socket, &response, sizeof(CalculatorResponse), 0,
                                      (struct sockaddr *)&server_addr, &server_len); // server_len updated by recvfrom
            if (bytes_received < 0) {
                perror("ERROR: recvfrom failed");
                break; // Exit loop on receive error
            }

            // Validate received response size
            if (bytes_received != sizeof(CalculatorResponse)) {
                fprintf(stderr, "WARNING: Received incomplete response (expected %lu bytes, got %zd).\n",
                        sizeof(CalculatorResponse), bytes_received);
                printf("Server response malformed.\n");
            } else {
                // 5. Display the result or error
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

    // 6. Close the client socket
    printf("Closing client socket.\n");
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