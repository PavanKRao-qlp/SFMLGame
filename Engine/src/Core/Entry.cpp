#pragma once
#include "EnginePCH.h"
#include "Core/App.h"

extern UMBRA::App* CreateApplication();

int main()
{
    UMBRA::App* app = CreateApplication();
    int returnCode = app->Bootup();
    delete app;
    return returnCode;
}
