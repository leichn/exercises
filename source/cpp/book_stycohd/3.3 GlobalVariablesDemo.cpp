#include <iostream>
using namespace std;

// Declare three global integers
int firstNumber = 0;
int secondNumber = 0;
int multiplicationResult = 0;

void MultiplyNumbers ()
{
   cout << "Enter the first number: ";
   cin >> firstNumber;

   cout << "Enter the second number: ";
   cin >> secondNumber;

   // Multiply two numbers, store multiplicationResult in a variable
   multiplicationResult = firstNumber * secondNumber;

   // Display multiplicationResult
   cout << "Displaying from MultiplyNumbers(): ";
   cout << firstNumber << " x " << secondNumber << " = " << multiplicationResult << endl;
}

int main ()
{
   cout << "This program will help you multiply two numbers" << endl;

   // Call the function that does all the work
   MultiplyNumbers();

   cout << "Displaying from main(): ";

   // This line will now compile and work!
   cout << firstNumber << " x " << secondNumber << " = " << multiplicationResult << endl;

   return 0;
}