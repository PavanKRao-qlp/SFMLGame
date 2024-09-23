#include "Core/App.h"
#include "Diag/Logger.h"
#include "Game/SimpleGameinstance.h"

class SimpleApp : public UMBRA::App
{
private:
    /* data */
public:
    SimpleApp();
    ~SimpleApp();
};

SimpleApp::SimpleApp(/* args */) : App(new SimpleGameInstance())
{
    // Logger::Log(LogType::Verbose, "SimpleApp Spawned");
}

SimpleApp::~SimpleApp()
{
}

UMBRA::App *CreateApplication()
{
    SimpleApp *app = new SimpleApp();
    return app;
}