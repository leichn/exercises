#include <string>
#include <iostream>
#include <algorithm>

int main()
{
	using namespace std;

	cout << "Please enter a string for case-conversion:" << endl;
	cout << "> ";

	string strInput;
	getline(cin, strInput);
	cout << endl;

	for (size_t nCharIndex = 0
		; nCharIndex < strInput.length()
		; nCharIndex += 2)
		strInput[nCharIndex] = toupper(strInput[nCharIndex]);

	cout << "The string converted to upper case is: " << endl;
	cout << strInput << endl << endl;

	return 0;
}
