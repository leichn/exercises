#include <iostream>
using namespace std;

class Mammal
{
public:
   void FeedBabyMilk()
   {
      cout << "Mammal: Baby says glug!" << endl;
   }

   Mammal()
   {
      cout << "Mammal: Contructor" << endl;
   }
   ~Mammal()
   {
      cout << "Mammal: Destructor" << endl;
   }
};

class Reptile
{
public:
   void SpitVenom()
   {
      cout << "Reptile: Shoo enemy! Spits venom!" << endl;
   }

   Reptile()
   {
      cout << "Reptile: Contructor" << endl;
   }
   ~Reptile()
   {
      cout << "Reptile: Destructor" << endl;
   }
};

class Bird
{
public:
   void LayEggs()
   {
      cout << "Bird: Laid my eggs, am lighter now!" << endl;
   }

   Bird()
   {
      cout << "Bird: Contructor" << endl;
   }
   ~Bird()
   {
      cout << "Bird: Destructor" << endl;
   }
};

class Platypus: public Mammal, public Bird, public Reptile
{
public:
   Platypus()
   {
      cout << "Platypus: Contructor" << endl;
   }
   ~Platypus()
   {
      cout << "Platypus: Destructor" << endl;
   }
};
   
int main() 
{
   Platypus realFreak;
   //realFreak.LayEggs();
   //realFreak.FeedBabyMilk();
   //realFreak.SpitVenom();

   return 0;
}