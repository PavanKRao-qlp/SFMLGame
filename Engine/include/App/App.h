#pragma once
#include <PakFile.h>
#include "App/AppWindow.h"

class App
{
private:
    class AppWindow *mAppWindow;
    // bool bCloseApp = true;

    // PakFile *P = nullptr;
    // char *data = nullptr;
    // uint64_t size = 0;
    /* data */
public:
    App(/* args */);
    ~App();
    bool Init();
    void Run();
    void Exit();
};
