#include <iostream>
using namespace std;

// Function declaration
int DemoConsoleOutput();

int main()
{
   // Function call
	DemoConsoleOutput();

	return 0;
}

// Function definition
int DemoConsoleOutput()
{
	cout << "This is a simple string literal" << endl;
	cout << "Writing number five: " << 5 << endl;
	cout << "Performing division 10 / 5 = " << 10 / 5 << endl;
	cout << "Pi when approximated is 22 / 7 = " << 22 / 7 << endl;
	cout << "Pi actually is 22 / 7 = " << 22.0 / 7 << endl;

	return 0;
}