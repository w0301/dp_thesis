#include "GuiRuntime.h"

using namespace std;

static shared_ptr<ExecObject> generateMessage(u32string name, u32string type) {
    auto msg = make_shared<ExecObject>();
    msg->setFieldByPath("name", make_shared<ExecString>(name));
    msg->setFieldByPath("type", make_shared<ExecString>(type));
    return msg;
}

static bool resultHasName(shared_ptr<ExecValue> res, u32string name) {
    auto obj = static_pointer_cast<ExecObject>(res);
    auto nameObj = static_pointer_cast<ExecString>(obj->getFieldByPath("name"));
    return nameObj->getValue() == name;
}

GuiRuntime::GuiRuntime(const std::string& filePath, Scheduler::Type type, int workers) :
        ProgramRuntime(filePath, type, workers) {
    // registering messages
    registerMessageGenerator("guiUpdate", 1, [] {
        return generateMessage(U"guiUpdate", U"gui");
    }, [](shared_ptr<ExecValue> res) {
        return resultHasName(res, U"guiUpdate");
    });

    registerMessageGenerator("dataUpdate", 500, [] {
        return generateMessage(U"dataUpdate", U"data");
    }, [](shared_ptr<ExecValue> res) {
        return resultHasName(res, U"dataUpdate");
    });

    registerMessageGenerator("realTimeDataUpdate", 50, [] {
        return generateMessage(U"realTimeDataUpdate", U"data");
    }, [](shared_ptr<ExecValue> res) {
        return resultHasName(res, U"realTimeDataUpdate");
    });

    registerMessageGenerator("renderRequest", 16, [] {
        return generateMessage(U"renderRequest", U"render");
    }, [](shared_ptr<ExecValue> res) {
        return resultHasName(res, U"renderRequest");
    });
}

shared_ptr<ExecObject> GuiRuntime::createInitMessage() const {
    return generateMessage(U"init", U"init");
}
