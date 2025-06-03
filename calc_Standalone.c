#include <stdio.h> 
#include <stdlib.h>

// --- Function Prototypes ---// Function to display the calculator menu
void display_menu();

// Arithmetic functions
double add(double num1, double num2);
double subtract(double num1, double num2);
double multiply(double num1, double num2);
double divide(double num1, double num2);

// --- Main Program ---
int main(void)
{
    int choice;
    double num1, num2, result;

    printf("Welcome to the Simple Calculator!\n");

    while (1) 
    {
        display_menu(); 
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input. Please enter a number.\n");
            // Clear input buffer to prevent infinite loop on bad input
            while (getchar() != '\n');
            continue; // Go back to the beginning of the loop
        }

        if (choice == 0) // Exit option
        {
            printf("Exiting calculator. Goodbye!\n");
            break;
        }
        else if (choice >= 1 && choice <= 4) 
        {
            printf("Enter first number: ");
            if (scanf("%lf", &num1) != 1)
            {
                printf("Invalid input. Please enter a valid number.\n");
                while (getchar() != '\n');
                continue;
            }

            printf("Enter second number: ");
            if (scanf("%lf", &num2) != 1)
            {
                printf("Invalid input. Please enter a valid number.\n");
                while (getchar() != '\n');
                continue;
            }

            // Perform the chosen operation
            switch (choice)
            {
                case 1: // Add
                    result = add(num1, num2);
                    printf("Result: %.2lf + %.2lf = %.2lf\n", num1, num2, result);
                    break;
                case 2: // Subtract
                    result = subtract(num1, num2);
                    printf("Result: %.2lf - %.2lf = %.2lf\n", num1, num2, result);
                    break;
                case 3: // Multiply
                    result = multiply(num1, num2);
                    printf("Result: %.2lf * %.2lf = %.2lf\n", num1, num2, result);
                    break;
                case 4: // Divide
                    if (num2 == 0) // Division by zero error
                    {
                        printf("Error: Division by zero is not allowed.\n");
                    }
                    else
                    {
                        result = divide(num1, num2);
                        printf("Result: %.2lf / %.2lf = %.2lf\n", num1, num2, result);
                    }
                    break;
            }
        }
        else // Invalid menu choice
        {
            printf("Invalid choice. Please enter a number between 0 and 4.\n");
        }
        printf("\n"); 
    }

    return EXIT_SUCCESS; 
}



void display_menu()
{
    printf("-------------------------\n");
    printf("     Simple Calculator\n");
    printf("-------------------------\n");
    printf("1. Add\n");
    printf("2. Subtract\n");
    printf("3. Multiply\n");
    printf("4. Divide\n");
    printf("0. Exit\n");
    printf("-------------------------\n");
}

// Performs addition
double add(double num1, double num2)
{
    return num1 + num2;
}

// Performs subtraction
double subtract(double num1, double num2)
{
    return num1 - num2;
}

// Performs multiplication
double multiply(double num1, double num2)
{
    return num1 * num2;
}

// Performs division
double divide(double num1, double num2)
{
    if (num2 == 0)
    {
        return 0.0; // Or handle error appropriately, e.g., via a status code
    }
    return num1 / num2;
}