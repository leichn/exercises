#include <iostream>
#include <iomanip>
using namespace std;

int main()
{
   cout << "Hey - default!" << endl;

   cout << setw(35);
   cout << "Hey - right aligned!" << endl;

   cout << setw(35) << setfill('*');
   cout << "Hey - right aligned!" << endl;

   cout << "Hey - back to default!" << endl;
   
   return 0;
}