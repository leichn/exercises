#include <iostream>
using namespace std;

class B
{
public:
    B()
    {
        cout << "default B constructor" << endl;
    }

    B(int a, int b)
    {
        x = a;
        y = b;
        cout << "overload B constructor: " << x << " " << y << endl;
    }

    void print()
    {
        printf("BASE private members: \n");
        printf("x = %d, y = %d\n", x, y);
    }

private:
    int x = 1;
    int y = 2;
    static int z;
};

class D : public B
{
public:
    D()
    {
        cout << "default D constructor" << endl;
    }

    D(int a, int b)// : B(1, 2)
    {
        u = a;
        v = b;
        cout << "overload D constructor" << endl;
    }

    void print()
    {
        printf("DERIVED private members: \n");
        printf("u = %d, v = %d\n", u, v);
        //B::print();
    }

private:
    int u = 11;
    int v = 22;
    static int w;
};

int main()
{
    B base(11, 22);
    B base1;
    D derived(111, 222);
    D derived1;

    //base = derived;
    //base.print();
    //derived.print();

    cout << "sizeof(B) = " << sizeof(base1) << endl;
    cout << "sizeof(D) = " << sizeof(derived1) << endl;

    return 0;
}