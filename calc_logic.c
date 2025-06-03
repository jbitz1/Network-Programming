/*
 * calc_logic.c - Core calculation logic for the Calculator application
 *
 * This file implements the arithmetic functions declared in calc_common.h.
 * It does not handle any networking or user interface aspects.
 */

#include "calc_common.h" // Include common definitions and function prototypes
#include <stdio.h>       // For potential debugging or specific output if needed
#include <math.h>        // Not strictly needed for basic ops, but good for general math


// --- Function Implementations for Calculator Logic ---

/*
 * Adds two double-precision floating-point numbers.
 * Parameters:
 * num1 - The first operand.
 * num2 - The second operand.
 * Returns:
 * The sum of num1 and num2.
 */
double add(double num1, double num2) {
    return num1 + num2;
}

/*
 * Subtracts the second number from the first.
 * Parameters:
 * num1 - The first operand (minuend).
 * num2 - The second operand (subtrahend).
 * Returns:
 * The difference (num1 - num2).
 */
double subtract(double num1, double num2) {
    return num1 - num2;
}

/*
 * Multiplies two double-precision floating-point numbers.
 * Parameters:
 * num1 - The first operand.
 * num2 - The second operand.
 * Returns:
 * The product of num1 and num2.
 */
double multiply(double num1, double num2) {
    return num1 * num2;
}

/*
 * Divides the first number by the second.
 * Parameters:
 * num1 - The dividend.
 * num2 - The divisor.
 * Returns:
 * The quotient (num1 / num2).
 * Note: Division by zero is typically handled by the caller (server)
 * based on the 'status' field in CalculatorResponse.
 */
double divide(double num1, double num2) {
    // In a real-world scenario, you might return a special value or set an error flag
    // here if this function could be called directly without prior zero-check.
    // However, as per our pseudocode, the server will check for num2 == 0 before calling.
    if (num2 == 0.0) {
        // This case should ideally be prevented by the caller,
        // but as a fallback, return 0.0 or a special value if it were an independent function
        // For this context, the server will handle the error status.
        return 0.0;
    }
    return num1 / num2;
}