#include "Core/App.h"

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
    printf("hello from SimpleApp!!\n");
}

SimpleApp::~SimpleApp()
{
}

D2D::App* CreateApplication() {
    printf("hello from CreateApplication!!\n");
 return new SimpleApp();
}