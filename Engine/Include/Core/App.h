#pragma once

namespace D2D
{
    class App
    {
    private:
        bool Init();
        void Run();
        int Exit();

        bool mAppRunning = false;

    public:
        App(/* args */);
        ~App();
        int Bootup();
    };
}