#include "App/App.h"
#include "Log/Logger.h"
#include "Game/GameInstance.h"

App::App(/* args */)
{
}

App::~App() {
}

bool App::Init()
{
  Logger::Log(LogType::Verbose, "Initializing App");
  mAppWindow = new AppWindow();
  mAppWindow->CreateWindow();
  //GameInstance::getInstance()->Initialize();
  return true;
}

void App::Run()
{
  while (!mAppWindow->bWindowClosed)
  {

    mAppWindow->Update();
    mAppWindow->ClearDisplay();
    // for each spriteComponent S Get tranform T
    {
      // Renderer->Draw(S)
    }
    mAppWindow->RefreshDisplay();
  }
  //  GameInstance::getInstance()->GetCurrentGameWorld()->GetSceneEntities();
}

void App::Exit()
{
  mAppWindow->CloseWindow();
  delete mAppWindow;
  //  GameInstance::getInstance()->ShutDown();
}
