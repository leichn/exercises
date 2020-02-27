#include <bitset>
#include <iostream>

int main()
{
	// Initialize the bitset to 1001
	std::bitset <4> fourBits(9);

	std::cout << "fourBits: " << fourBits << std::endl;

	// Initialize another bitset to 0010
	std::bitset <4> fourMoreBits(2);

	std::cout << "fourMoreBits: " << fourMoreBits << std::endl;

	std::bitset<4> addResult(fourBits.to_ulong() + fourMoreBits.to_ulong());
	std::cout << "The result of the addition is: " << addResult;

	return 0;
}
