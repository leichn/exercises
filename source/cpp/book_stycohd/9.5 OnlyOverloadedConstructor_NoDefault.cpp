#include <iostream>
#include <string>
using namespace std;

class Human
{
private:
   string name;
   int age;

public:
   Human(string humansName /*= "Adam"*/, int humansAge /*= 25*/) 
   {
      name = humansName;
      age = humansAge;
      cout << "Overloaded constructor creates " << name;
      cout << " of age " << age << endl;
   }

   void IntroduceSelf()
   {
      cout << "I am " + name << " and am ";
      cout << age << " years old" << endl;
   }
};

int main()
{
   Human firstMan("Adam", 25); 
   Human firstWoman("Eve", 28);

   firstMan.IntroduceSelf();
   firstWoman.IntroduceSelf();
}