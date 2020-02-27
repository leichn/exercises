#include <string>
#include <iostream>
#include <algorithm>

int main()
{
	using namespace std;

	cout << "Please enter a word for palindrome-check:" << endl;
	string strInput;
	cin >> strInput;

	string strCopy(strInput);
	reverse(strCopy.begin(), strCopy.end());

	if (strCopy == strInput)
		cout << strInput << " is a palindrome!" << endl;
	else
		cout << strInput << " is not a palindrome." << endl;

	return 0;
}
