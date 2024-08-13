#include "Core/App.h"
#include "Diag/Logger.h"

class SimpleApp : public D2D::App
{
private:
    /* data */
public:
    SimpleApp(/* args */);
    ~SimpleApp();
};

SimpleApp::SimpleApp(/* args */)
{
    //Logger::Log(LogType::Verbose, "SimpleApp Spawned");
}

SimpleApp::~SimpleApp()
{
}

D2D::App* CreateApplication() {
 return new SimpleApp();
}