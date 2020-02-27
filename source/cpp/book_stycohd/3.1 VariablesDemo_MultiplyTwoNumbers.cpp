#include <iostream>
using namespace std;

int main ()
{
   cout << "This program will help you multiply two numbers" << endl;

   cout << "Enter the first number: ";
   int firstNumber = 0;
   cin >> firstNumber;

   cout << "Enter the second number: ";
   int secondNumber = 0;
   cin >> secondNumber;

   // Multiply two numbers, store result in a variable
   int multiplicationResult = firstNumber * secondNumber;

	// Display result
	cout << firstNumber << " x " << secondNumber;
	cout << " = " << multiplicationResult << endl;
   
   return 0;
}