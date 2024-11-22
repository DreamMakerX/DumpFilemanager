#pragma once
#include <iostream>
#include "DumpFileManager.h"

void DoSomething()
{
	int count = 0;
	while (count < 10)
	{
		Sleep(1000);
		std::cout << "get tick count: " << count + 1 << std::endl;
		++count;
	}
	throw "something error";
	Sleep(10 * 1000);
	std::cout << "DoSomething success"<< std::endl;
}
int main()
{
	StartDetectCrash();
	DoSomething();
	return 0;
}