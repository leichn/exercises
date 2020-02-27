#include <string>
#include <iostream>

using namespace std;

// Find the number of character 'chToFind' in string "strInput"
int GetNumCharacters(string& strInput, char chToFind)
{
	int nNumCharactersFound = 0;

	size_t nCharOffset = strInput.find(chToFind);
	while (nCharOffset != string::npos)
	{
		++nNumCharactersFound;

		nCharOffset = strInput.find(chToFind, nCharOffset + 1);
	}

	return nNumCharactersFound;
}

int main()
{

	cout << "Please enter a string:" << endl << "> ";
	string strInput;
	getline(cin, strInput);

	int nNumVowels = GetNumCharacters(strInput, 'a');
	nNumVowels += GetNumCharacters(strInput, 'e');
	nNumVowels += GetNumCharacters(strInput, 'i');
	nNumVowels += GetNumCharacters(strInput, 'o');
	nNumVowels += GetNumCharacters(strInput, 'u');

	// DIY: handle capitals too..

	cout << "The number of vowels in that sentence is:" << nNumVowels;

	return 0;
}
