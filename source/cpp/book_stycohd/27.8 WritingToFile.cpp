#include<fstream>
#include<iostream>
using namespace std;

int main()
{
   ofstream myFile;
   myFile.open("HelloFile.txt", ios_base::out);

   if (myFile.is_open())
   {
      cout << "File open successful" << endl;

      myFile << "My first text file!" << endl;
      myFile << "Hello file!";

      cout << "Finished writing to file, will close now" << endl;
      myFile.close();
   }

   return 0;
}
