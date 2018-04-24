#include <string>
#include <iostream>

#include "GuiRuntime.h"
#include "TestRuntime.h"
#include "ServerRuntime.h"
#include "SimpleProgramRuntime.h"

using namespace std;

void runSchedulerTest(int msgsCount, int varsCount) {
    auto messages = TestMessage::generateMessages(msgsCount, varsCount, 1.0);

    double ref = TestRuntime(Scheduler::RWLocking, 1, varsCount, messages).run(-1);

    TestRuntime(Scheduler::RWLocking, 2, varsCount, messages).run(ref);
    TestRuntime(Scheduler::RWLocking, 4, varsCount, messages).run(ref);

    TestRuntime(Scheduler::WLocking, 2, varsCount, messages).run(ref);
    TestRuntime(Scheduler::WLocking, 4, varsCount, messages).run(ref);
}

void runServerTest(int seconds) {
    cout << ">>>>>> Testing 1 worker for " << seconds << " seconds:" << endl;
    ServerRuntime("codes/Server.lang", Scheduler::RWLocking, 1).run(seconds * 1000);

    cout << ">>>>>> Testing 2 workers for " << seconds << " seconds:" << endl;
    ServerRuntime("codes/Server.lang", Scheduler::RWLocking, 2).run(seconds * 1000);

    cout << ">>>>>> Testing 4 workers for " << seconds << " seconds:" << endl;
    ServerRuntime("codes/Server.lang", Scheduler::RWLocking, 4).run(seconds * 1000);
}

void runGuiTest(int seconds) {
    cout << ">>>>>> Testing 1 worker for " << seconds << " seconds:" << endl;
    GuiRuntime("codes/Gui.lang", Scheduler::WLocking, 1).run(seconds * 1000);

    cout << ">>>>>> Testing 2 workers for " << seconds << " seconds:" << endl;
    GuiRuntime("codes/Gui.lang", Scheduler::WLocking, 2).run(seconds * 1000);

    cout << ">>>>>> Testing 4 workers for " << seconds << " seconds:" << endl;
    GuiRuntime("codes/Gui.lang", Scheduler::WLocking, 4).run(seconds * 1000);
}

void runInterpreter(const string& programPath, const string& strArg) {
    SimpleProgramRuntime runtime(programPath);

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
        int seconds = (argc > 2 ? stoi(argv[2]) : 10);
        runServerTest(seconds);
    }
    else if (argc > 1 && string(argv[1]) == "--test-gui") {
        int seconds = (argc > 2 ? stoi(argv[2]) : 10);
        runGuiTest(seconds);
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
