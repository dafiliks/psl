#include <string>
#include <iostream>

std::string test(std::string a)
{
	std::string k = std::string("hey") + a;
	k = std::string("damn");
	std::cout << k;
	return k;
}

int main()
{
	double k = 200;
	double y = test(std::string("a"));
	return 0;
}
