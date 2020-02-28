#include <iostream>

using namespace std;

class Fish
{
public:
    Fish()
    {
        cout << "Constructed Fish" << endl;
    }
    virtual ~Fish()
    {
        cout << "Destroyed Fish" << endl;
    }
};

class Tuna : public Fish
{
public:
    Tuna()
    {
        cout << "Constructed Tuna" << endl;
    }
    ~Tuna()
    {
        cout << "Destroyed Tuna" << endl;
    }
};

void DeleteFishMemory(Fish *pFish)
{
    // 若第 12 行删除 virtual 关键字，则传入实参是派生类对象时，也被当作其基类对象进行析构
    // 若第 12 行携带 virtual 关键字，则传入实参是派生类对象时，将被当作派生类对象进行析构
    delete pFish;
}

int main()
{
    cout << "Allocating a Tuna on the free store:" << endl;
    Tuna *pTuna = new Tuna;
    cout << "Deleting the Tuna: " << endl;
    DeleteFishMemory(pTuna);

    cout << "Instantiating a Tuna on the stack:" << endl;
    Tuna tuna;
    cout << "Automatic destruction as it goes out of scope: " << endl;

    return 0;
}