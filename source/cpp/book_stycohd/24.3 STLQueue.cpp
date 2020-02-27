#include <queue>
#include <list>

int main ()
{
   using namespace std;

   // A queue of integers
   queue <int> numsInQ;

   // A queue of doubles
   queue <double> dblsInQ;

   // A queue of doubles stored internally in a list
   queue <double, list <double> > dblsInQInList;

   // one queue created as a copy of another
   queue<int> copyQ(numsInQ);

   return 0;
}