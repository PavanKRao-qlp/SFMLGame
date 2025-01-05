#pragma once

template <class T>
class Singleton {

public:
    /* Prevent copy and assignment */
    Singleton(const Singleton&)            = delete;
    Singleton& operator=(const Singleton&) = delete;
    /* Static access method. */
    inline static T* GetInstance() {
        if (mInstance == nullptr) {
            mInstance = new T();
        }
        return mInstance;
    }

    Singleton() {};
    ~Singleton() {}

private:
    /* Here will be the instance stored. */
    inline static T* mInstance = 0;
    /* Private constructor to prevent instancing. */
};
