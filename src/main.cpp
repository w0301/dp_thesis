#include <string>
#include <iostream>

#include "Runtime.h"
#include "TestRuntime.h"

using namespace std;

void runSchedulerTest() {
  TestRuntime().runTests();
}

void runServerTest() {
    // TODO
}

void runGuiTest() {
    // TODO
}

void runInterpreter(string programPath, string strArg) {
    Runtime runtime(programPath.c_str());

    // setup arg for main
    auto arg = make_shared<ExecString>();
    arg->setValueUTF8(strArg);

    // call interpreter
    auto res = runtime.exec(arg);

    // print result
    cout << res->toString() << endl;
}

int main(int argc, char *argv[]) {
    if (argc > 1 && string(argv[1]) == "--test-scheduler") {
        runSchedulerTest();
    }
    else if (argc > 1 && string(argv[1]) == "--test-server") {
        runServerTest();
    }
    else if (argc > 1 && string(argv[1]) == "--test-gui") {
        runGuiTest();
    }
    else if (argc > 1) {
        runInterpreter(string(argv[1]), argc > 2 ? string(argv[2]) : "");
    }
    else {
        // fallback
        runInterpreter("../codes/Test.lang", "argument");
    }

    return 0;
}
