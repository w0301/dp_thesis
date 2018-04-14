#include <string>
#include <iostream>

#include "SimpleProgramRuntime.h"
#include "TestRuntime.h"

using namespace std;

void runSchedulerTest(int msgsCount, int varsCount) {
    auto messages = TestMessage::generateMessages(msgsCount, varsCount, 1.0);

    double ref = TestRuntime(Scheduler::RWLocking, 1, varsCount, messages).run(-1);

    TestRuntime(Scheduler::RWLocking, 2, varsCount, messages).run(ref);
    TestRuntime(Scheduler::RWLocking, 4, varsCount, messages).run(ref);

    TestRuntime(Scheduler::WLocking, 2, varsCount, messages).run(ref);
    TestRuntime(Scheduler::WLocking, 4, varsCount, messages).run(ref);
}

void runServerTest() {
    // TODO
}

void runGuiTest() {
    // TODO
}

void runInterpreter(string programPath, string strArg) {
    SimpleProgramRuntime runtime(programPath.c_str());

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
        int msgsCount = (argc > 2 ? stoi(argv[2]) : 1000);
        int varsCount = (argc > 3 ? stoi(argv[3]) : 10);
        runSchedulerTest(msgsCount, varsCount);
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
