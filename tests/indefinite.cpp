#include <iostream>

int main()
{
    int a = 1;
    do
    {
    std::cout << (a);
    a = a+1;
    } while (!(a==4));
    a = 1;
    while (a<4)
    {
    std::cout << (a);
    a = a+1;
    }
    return 0;
}
