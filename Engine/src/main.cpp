#include "App/App.h"
#include <iostream>

int main() { 
    std::cout << "Hello World!\n";
    App* app = new App();
    app->Init();
    app->Run();
    app->Exit();
    delete app;   
    return 0;
}
