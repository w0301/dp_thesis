#include <iostream>

#include "Runtime.h"
#include "TestRuntime.h"

using namespace std;

void runSchedulerTest() {
  TestRuntime().runTests();
}

void runInterpreterTest() {
    Runtime runtime("codes/Test.lang");

    // setup arg for main
    auto arg = make_shared<ExecString>();
    arg->setValue(U"Test arg for main function");

    // call interpreter
    auto res = runtime.exec(arg);

    // print result
    cout << "Main return value: " << endl;
    cout << res->toString() << endl;
}

int main() {
    //runSchedulerTest();

    runInterpreterTest();

    return 0;
}
