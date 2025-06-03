/*
 * calc_common.h - Common definitions for the Calculator application
 *
 * This header file defines the data structures and enumerations
 * used for communication between the calculator client and server,
 * as well as by the core calculation logic.
 */

#ifndef CALC_COMMON_H
#define CALC_COMMON_H

// Enum for the type of arithmetic operation
typedef enum {
    ADD = 1,
    SUBTRACT = 2,
    MULTIPLY = 3,
    DIVIDE = 4
} OperationType;

// Structure for a calculator request from client to server
typedef struct {
    OperationType operation; // The type of operation to perform
    double num1;             // The first operand
    double num2;             // The second operand
} CalculatorRequest;

// Structure for a calculator response from server to client
typedef struct {
    int status;   // 0 for success, -1 for error (e.g., division by zero)
    double result; // The result of the operation if successful
} CalculatorResponse;

// --- Function Prototypes for Calculator Logic (to be implemented in calc_logic.c) ---
// These prototypes are included here so calc_server and calc_client can see them,
// if they were to directly link with calc_logic.c.
// For client-server, the logic functions are called only by the server.
// However, including them here provides a clear API for the calculator functions.
double add(double num1, double num2);
double subtract(double num1, double num2);
double multiply(double num1, double num2);
double divide(double num1, double num2);

#endif // CALC_COMMON_H