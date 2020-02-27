#include <iostream>
using namespace std;

class Animal
{
public:
   Animal()
   {
      cout << "Animal constructor" << endl;
   }

   // sample member
   int age;
};

class Mammal:public virtual Animal
{
};

class Bird:public virtual Animal
{
};

class Reptile:public virtual Animal
{
};

class Platypus final:public Mammal, public Bird, public Reptile
{
public:
   Platypus()
   {
      cout << "Platypus constructor" << endl;
   }
};

int main()
{
   Platypus duckBilledP;

   // no compile error as there is only one Animal::age
   duckBilledP.age = 25; 

   return 0;
}