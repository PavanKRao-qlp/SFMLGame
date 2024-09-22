#include "Core/App.h"
#include "Diag/Logger.h"
#include "Game/SimpleGameinstance.h"

class SimpleApp : public D2D::App
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

D2D::App *CreateApplication()
{
    SimpleApp *app = new SimpleApp();
    return app;
}