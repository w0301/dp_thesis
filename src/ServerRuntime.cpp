#include "ServerRuntime.h"

using namespace std;

ServerRuntime::ServerRuntime(const std::string& filePath, Scheduler::Type type, int workers) :
        ProgramRuntime(filePath, type, workers) {
    // registering messages
    registerMessageGenerator("xInc", 2, [] {
        auto msg = make_shared<ExecObject>();
        msg->setFieldByPath("type", make_shared<ExecString>(U"xInc"));
        return msg;
    }, [](shared_ptr<ExecValue> res) {
        auto obj = static_pointer_cast<ExecObject>(res);
        auto type = static_pointer_cast<ExecString>(obj->getFieldByPath("type"));
        return type->getValue() == U"xInc";
    });

    registerMessageGenerator("yInc", 3, [] {
        auto msg = make_shared<ExecObject>();
        msg->setFieldByPath("type", make_shared<ExecString>(U"yInc"));
        return msg;
    }, [](shared_ptr<ExecValue> res) {
        auto obj = static_pointer_cast<ExecObject>(res);
        auto type = static_pointer_cast<ExecString>(obj->getFieldByPath("type"));
        return type->getValue() == U"yInc";
    });

    registerMessageGenerator("zInc", 5, [] {
        auto msg = make_shared<ExecObject>();
        msg->setFieldByPath("type", make_shared<ExecString>(U"zInc"));
        return msg;
    }, [](shared_ptr<ExecValue> res) {
        auto obj = static_pointer_cast<ExecObject>(res);
        auto type = static_pointer_cast<ExecString>(obj->getFieldByPath("type"));
        return type->getValue() == U"zInc";
    });
}

shared_ptr<ExecObject> ServerRuntime::createInitMessage() const {
    auto msg = make_shared<ExecObject>();
    msg->setFieldByPath("type", make_shared<ExecString>(U"init"));
    return msg;
}
