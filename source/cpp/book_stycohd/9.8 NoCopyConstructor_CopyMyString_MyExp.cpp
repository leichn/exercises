#include <iostream>
#include <string.h>
#include <stdio.h>

using namespace std;

class MyString
{
private:
    char *m_buffer;

public:
    MyString(const char *initString)        // Constructor
    {
        m_buffer = NULL;
        cout << "constructor" << endl;
        if (initString != NULL)
        {
            m_buffer = new char[strlen(initString) + 1];
            strcpy(m_buffer, initString);
        }
    }

    ~MyString()                             // Destructor
    {
        cout << "destructor, delete m_buffer " << hex << (unsigned int *)m_buffer << dec << endl;
        delete[] m_buffer;
    }

    void PrintAddress(const string &prefix)
    {
        cout << prefix << hex << (unsigned int *)m_buffer << dec << endl;
    }
};

void UseMyString(MyString str)
{
    str.PrintAddress("str.m_buffer addr: ");
}

int main()
{
    MyString test("12345678901234567890");     // 直接初始化，执行构造函数 MyString(const char* initString)
    test.PrintAddress("test.m_buffer addr: ");
    UseMyString(test);                         // 拷贝初始化，执行合成的默认拷贝构造函数，浅复制

    return 0;
}
