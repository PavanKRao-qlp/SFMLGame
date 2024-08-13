#pragma once
template<class T>
class Singleton
{
    private:
        /* Here will be the instance stored. */
        inline static T* mInstance = 0;
        /* Private constructor to prevent instancing. */
    protected:
        inline Singleton() { };

    public:
        /* Static access method. */
        inline static T* getInstance() {
            if (mInstance == nullptr)   mInstance = new T();
            return mInstance;
        }
};