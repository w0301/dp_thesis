#include <iostream>

#include "Runtime.h"
#include "TestRuntime.h"

using namespace std;

void runSchedulerTest() {
  TestRuntime().runTests();
}

void runInterpreterTest() {
    Runtime runtime("../codes/Test.lang");
}

int main() {
    //runSchedulerTest();

    runInterpreterTest();

    return 0;
}
