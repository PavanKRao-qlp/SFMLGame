#pragma once
#include "EnginePCH.h"
#include "Core/Event.h"

namespace UMBRA
{
    namespace KeyBoard
    {
        enum Keycode : int16
        {
            Unknown = -1, //!< Unhandled key
            A = 0,        //!< The A key
            B,            //!< The B key
            C,            //!< The C key
            D,            //!< The D key
            E,            //!< The E key
            F,            //!< The F key
            G,            //!< The G key
            H,            //!< The H key
            I,            //!< The I key
            J,            //!< The J key
            K,            //!< The K key
            L,            //!< The L key
            M,            //!< The M key
            N,            //!< The N key
            O,            //!< The O key
            P,            //!< The P key
            Q,            //!< The Q key
            R,            //!< The R key
            S,            //!< The S key
            T,            //!< The T key
            U,            //!< The U key
            V,            //!< The V key
            W,            //!< The W key
            X,            //!< The X key
            Y,            //!< The Y key
            Z,            //!< The Z key
            Num0,         //!< The 0 key
            Num1,         //!< The 1 key
            Num2,         //!< The 2 key
            Num3,         //!< The 3 key
            Num4,         //!< The 4 key
            Num5,         //!< The 5 key
            Num6,         //!< The 6 key
            Num7,         //!< The 7 key
            Num8,         //!< The 8 key
            Num9,         //!< The 9 key
            Escape,       //!< The Escape key
            LControl,     //!< The left Control key
            LShift,       //!< The left Shift key
            LAlt,         //!< The left Alt key
            LSystem,      //!< The left OS specific key: window (Windows and Linux), apple (macOS), ...
            RControl,     //!< The right Control key
            RShift,       //!< The right Shift key
            RAlt,         //!< The right Alt key
            RSystem,      //!< The right OS specific key: window (Windows and Linux), apple (macOS), ...
            Menu,         //!< The Menu key
            LBracket,     //!< The [ key
            RBracket,     //!< The ] key
            Semicolon,    //!< The ; key
            Comma,        //!< The , key
            Period,       //!< The . key
            Apostrophe,   //!< The ' key
            Slash,        //!< The / key
            Backslash,    //!< The \ key
            Grave,        //!< The ` key
            Equal,        //!< The = key
            Hyphen,       //!< The - key (hyphen)
            Space,        //!< The Space key
            Enter,        //!< The Enter/Return keys
            Backspace,    //!< The Backspace key
            Tab,          //!< The Tabulation key
            PageUp,       //!< The Page up key
            PageDown,     //!< The Page down key
            End,          //!< The End key
            Home,         //!< The Home key
            Insert,       //!< The Insert key
            Delete,       //!< The Delete key
            Add,          //!< The + key
            Subtract,     //!< The - key (minus, usually from numpad)
            Multiply,     //!< The * key
            Divide,       //!< The / key
            Left,         //!< Left arrow
            Right,        //!< Right arrow
            Up,           //!< Up arrow
            Down,         //!< Down arrow
            Numpad0,      //!< The numpad 0 key
            Numpad1,      //!< The numpad 1 key
            Numpad2,      //!< The numpad 2 key
            Numpad3,      //!< The numpad 3 key
            Numpad4,      //!< The numpad 4 key
            Numpad5,      //!< The numpad 5 key
            Numpad6,      //!< The numpad 6 key
            Numpad7,      //!< The numpad 7 key
            Numpad8,      //!< The numpad 8 key
            Numpad9,      //!< The numpad 9 key
            F1,           //!< The F1 key
            F2,           //!< The F2 key
            F3,           //!< The F3 key
            F4,           //!< The F4 key
            F5,           //!< The F5 key
            F6,           //!< The F6 key
            F7,           //!< The F7 key
            F8,           //!< The F8 key
            F9,           //!< The F9 key
            F10,          //!< The F10 key
            F11,          //!< The F11 key
            F12,          //!< The F12 key
            F13,          //!< The F13 key
            F14,          //!< The F14 key
            F15,          //!< The F15 key
            Pause,        //!< The Pause key

            COUNT //!< Keep last -- the total number of keyboard keys
        };
    }
    namespace Mouse
    {
        enum MouseButton : int16
        {
            Left,     //!< The left mouse button
            Right,    //!< The right mouse button
            Middle,   //!< The middle (wheel) mouse button
            XButton1, //!< The first extra mouse button
            XButton2, //!< The second extra mouse button

            COUNT //!< Keep last -- the total number of mouse buttons
        };
    }

    class KeyPressedEvent : Event
    {
    public:
        inline KeyPressedEvent(KeyBoard::Keycode _code) { Key = _code; }
        KeyBoard::Keycode Key;
    };
    class KeyReleasedEvent : Event
    {
    public:
        inline KeyReleasedEvent(KeyBoard::Keycode _code) { Key = _code; }
        KeyBoard::Keycode Key;
    };
    class MouseButtonPressedEvent : Event
    {
    public:
        inline MouseButtonPressedEvent(Mouse::MouseButton _code, int16 _x, int16 _y)
        {
            Button = _code;
            x = _x;
            y = _y;
        }
        Mouse::MouseButton Button;
        int16 x = 0;
        int16 y = 0;
    };
    class MouseButtonReleasedEvent : Event
    {
    public:
        inline MouseButtonReleasedEvent(Mouse::MouseButton _code, int16 _x, int16 _y)
        {
            Button = _code;
            x = _x;
            y = _y;
        }
        Mouse::MouseButton Button;
        int16 x = 0;
        int16 y = 0;
    };
    class MouseMovedEvent : Event
    {
    public:
        inline MouseMovedEvent(int16 _x, int16 _y)
        {
            x = _x;
            y = _y;
        }
        int16 x = 0;
        int16 y = 0;
    };

    class Input
    {
    public:
        Input();
        ~Input();
        inline static void Update()
        {
            PrevMouseButtonPressedState = MouseButtonPressedState;
        }
        // static inline void Update() { PrevKeyPressedState = KeyPressedState; };
        /** Returns true while the user holds down the key identified by name. */
        static bool GetKey(KeyBoard::Keycode _code);
        /** Returns true during the frame the user starts pressing down the key identified by name. */
        static bool GetKeyDown(KeyBoard::Keycode _code);
        /** Returns true during the frame the user releases the key identified by name. */
        static bool GetKeyUp(KeyBoard::Keycode _code);
        /** Returns true during the frame the user releases the given mouse button.*/
        static bool GetMouseButton(Mouse::MouseButton _code);
        /** Returns true during the frame the user pressed the given mouse button. */
        static bool GetMouseButtonDown(Mouse::MouseButton _code);
        /*Returns true during the frame the user releases the given mouse button.*/
        static bool GetMouseButtonUp(Mouse::MouseButton _code);
        static int16 GetMousePositionX();
        static int16 GetMousePositionY();

    private:
        static void OnKeyPressed(const KeyPressedEvent &event);
        static void OnKeyReleased(const KeyReleasedEvent &event);
        static void OnMouseButtonPressed(const MouseButtonPressedEvent &event);
        static void OnMouseButtonReleased(const MouseButtonReleasedEvent &event);
        static void OnMouseMoved(const MouseMovedEvent &event);
        // BitField<KeyBoard::Keycode::COUNT> KeyPressedState;
        static inline BitField<KeyBoard::Keycode::COUNT> KeyPressedState = 0;
        static inline BitField<KeyBoard::Keycode::COUNT> PrevKeyPressedState = 0;
        static inline BitField<Mouse::MouseButton::COUNT> MouseButtonPressedState = 0;
        static inline BitField<Mouse::MouseButton::COUNT> PrevMouseButtonPressedState = 0;
        static inline int16 MouseX = 0;
        static inline int16 MouseY = 0;
    };

}