#pragma once

template <class T>
class Singleton {

public:
    /* Prevent copy and assignment */
    Singleton(const Singleton&)            = delete;
    Singleton& operator=(const Singleton&) = delete;
    /* Static access method. */
    static T* GetInstance() {
        if (mInstance == nullptr) {
            mInstance = new T();
        }
        return mInstance;
    }
    static void Destroy() {
        if (mInstance) {
            delete mInstance;
        }
    }

    Singleton() {}
    virtual ~Singleton() {}

private:
    /* Here will be the instance stored. */
    inline static T* mInstance = 0;
    /* Private constructor to prevent instancing. */
};

template <class T>
struct SingletonDestructor {
    void operator()(T* p) const {
        delete p;
    }
};
