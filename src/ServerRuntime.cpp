#include <cmath>
#include <cstdlib>

#include "ServerRuntime.h"

using namespace std;

shared_ptr<ExecObject> generateMessage(u32string name, u32string type, u32string reads, u32string writes) {
    auto msg = make_shared<ExecObject>();
    msg->setFieldByPath("name", make_shared<ExecString>(name));
    msg->setFieldByPath("type", make_shared<ExecString>(type));
    msg->setFieldByPath("reads", make_shared<ExecString>(reads));
    msg->setFieldByPath("writes", make_shared<ExecString>(writes));
    return msg;
}

bool resultHasName(shared_ptr<ExecValue> res, u32string name) {
    auto obj = static_pointer_cast<ExecObject>(res);
    auto nameObj = static_pointer_cast<ExecString>(obj->getFieldByPath("name"));
    return nameObj->getValue() == name;
}

u32string getRandomString(int length, int onesCount) {
    onesCount = min(length, onesCount);
    u32string res(length, U'0');

    while (onesCount) {
        int i = rand() % length;
        if (res[i] != U'1') {
            res[i] = U'1';
            onesCount -= 1;
        }
    }

    return res;
}

ServerRuntime::ServerRuntime(const std::string& filePath, Scheduler::Type type, int workers) :
        ProgramRuntime(filePath, type, workers) {
    // registering read messages
    registerMessageGenerator("readOneRandom", 10, [] {
        return generateMessage(U"readOneRandom", U"work", getRandomString(4, 1), U"0000");
    }, [](shared_ptr<ExecValue> res) {
        return resultHasName(res, U"readOneRandom");
    });

    registerMessageGenerator("readTwoRandom", 20, [] {
        return generateMessage(U"readTwoRandom", U"work", getRandomString(4, 2), U"0000");
    }, [](shared_ptr<ExecValue> res) {
        return resultHasName(res, U"readTwoRandom");
    });

    registerMessageGenerator("readAll", 50, [] {
        return generateMessage(U"readAll", U"work", U"1111", U"0000");
    }, [](shared_ptr<ExecValue> res) {
        return resultHasName(res, U"readAll");
    });

    // registering write messages
    registerMessageGenerator("writeOneRandom", 30, [] {
        return generateMessage(U"writeOneRandom", U"work", U"0000", getRandomString(4, 1));
    }, [](shared_ptr<ExecValue> res) {
        return resultHasName(res, U"writeOneRandom");
    });
}

shared_ptr<ExecObject> ServerRuntime::createInitMessage() const {
    return generateMessage(U"init", U"init", U"0000", U"0000");
}
