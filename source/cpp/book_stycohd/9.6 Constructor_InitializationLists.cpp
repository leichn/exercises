#include <iostream>
#include <string>
using namespace std;

class Human
{
private:
   int age;
   string name;

public:
   Human(string humansName = "Adam", int humansAge = 25)
        :name(humansName), age(humansAge)
   {
      cout << "Constructed a human called " << name;
      cout << ", " << age << " years old" << endl;
   }
};

int main()
{
   Human adam;
   Human eve("Eve", 18);

   return 0;
}