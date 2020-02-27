#include <vector>
#include <iostream>

using namespace std;

char DisplayOptions()
{
	cout << "What would you like to do?" << endl;
	cout << "Select 1: To enter an integer" << endl;
	cout << "Select 2: Query a value given an index" << endl;
	cout << "Select 3: To display the vector" << endl << "> ";
	cout << "Select 4: To quit!" << endl << "> ";

	char ch;
	cin >> ch;

	return ch;
}

int main()
{
	vector <int> vecData;

	char chUserChoice = '\0';
	while ((chUserChoice = DisplayOptions()) != '4')
	{
		if (chUserChoice == '1')
		{
			cout << "Please enter an integer to be inserted: ";
			int nDataInput = 0;
			cin >> nDataInput;

			vecData.push_back(nDataInput);
		}
		else if (chUserChoice == '2')
		{
			cout << "Please enter an index between 0 and ";
			cout << (vecData.size() - 1) << ": ";
			size_t index = 0;
			cin >> index;

			if (index < (vecData.size()))
			{
				cout << "Element [" << index << "] = " << vecData[index];
				cout << endl;
			}
		}
		else if (chUserChoice == '3')
		{
			cout << "The contents of the vector are: ";
			for (size_t index = 0; index < vecData.size(); ++index)
				cout << vecData[index] << ' ';
			cout << endl;
		}
	}
	return 0;
}
