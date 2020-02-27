#include <string>
#include <iostream>

int main()
{
	using namespace std;

	const string str1 = "I";
	const string str2 = "Love";
	const string str3 = "STL";
	const string str4 = "String.";

	string strResult = str1 + " " + str2 + " " + str3 + " " + str4;

	cout << "The sentence reads:" << endl;
	cout << strResult;

	return 0;
}