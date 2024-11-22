// test.cpp: 定义应用程序的入口点。
//

#include <iostream>
#include "DumpFileManager.h"

void doSomething() {
	std::cout << "doSomething start" << std::endl;

	Sleep(3 * 1000);

	throw "something error";

	Sleep(3 * 1000);

	std::cout << "doSomething success" << std::endl;
}
void main() {
	startDetectCrash();

	doSomething();
}