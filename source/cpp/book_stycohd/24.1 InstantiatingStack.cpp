#include <stack>
#include <vector>

int main ()
{
   using namespace std;

   // A stack of integers
   stack <int> numsInStack;

   // A stack of doubles
   stack <double> dblsInStack;

   // A stack of doubles contained in a vector
   stack <double, vector <double> > doublesStackedInVec;

   // initializing one stack to be a copy of another
   stack <int> numsInStackCopy(numsInStack);

   return 0;
}
