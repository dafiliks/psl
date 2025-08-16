#include <iostream>
#include <string>

void showAdd(int a, int b)
{
	int result = a+b;
	std::cout << result;
}

void sayHi()
{
	std::cout << std::string("Hi");
}

int add(int a, int b)
{
	int result = a+b;
	return result;
}

int main()
{
	
	
	
	showAdd(2, 3);
	int answer = add(2, 3)*6;
	return 0;
}
