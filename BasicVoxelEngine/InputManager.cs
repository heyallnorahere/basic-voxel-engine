using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct KeyState
    {
        public bool Up { get; set; }
        public bool Down { get; set; }
        public bool Held { get; set; }
    }
    ///<summary>
    /// probably unnecessary
    ///</summary>
    public enum Key
    {
        UNKNOWN = -1,
        SPACE = 32,
        APOSTROPHE = 39,
        COMMA = 44,
        MINUS = 45,
        PERIOD = 46,
        SLASH = 47,
        N0 = 48,
        N1 = 49,
        N2 = 50,
        N3 = 51,
        N4 = 52,
        N5 = 53,
        N6 = 54,
        N7 = 55,
        N8 = 56,
        N9 = 57,
        SEMICOLON = 59,
        EQUAL = 61,
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        LEFT_BRACKET = 91,
        BACKSLASH = 92,
        RIGHT_BRACKET = 93,
        GRAVE_ACCENT = 96,
        WORLD_1 = 161,
        WORLD_2 = 162,
        ESCAPE = 256,
        ENTER = 257,
        TAB = 258,
        BACKSPACE = 259,
        INSERT = 260,
        DELETE = 261,
        RIGHT = 262,
        LEFT = 263,
        DOWN = 264,
        UP = 265,
        PAGE_UP = 266,
        PAGE_DOWN = 267,
        HOME = 268,
        END = 269,
        CAPS_LOCK = 280,
        SCROLL_LOCK = 281,
        NUM_LOCK = 282,
        PRINT_SCREEN = 283,
        PAUSE = 284,
        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,
        // lets just skip all other F keys
        KP_0 = 320,
        KP_1 = 321,
        KP_2 = 322,
        KP_3 = 323,
        KP_4 = 324,
        KP_5 = 325,
        KP_6 = 326,
        KP_7 = 327,
        KP_8 = 328,
        KP_9 = 329,
        KP_DECIMAL = 330,
        KP_DIVIDE = 331,
        KP_MULTIPLY = 332,
        KP_SUBTRACT = 333,
        KP_ADD = 334,
        KP_ENTER = 335,
        KP_EQUAL = 336,
        LEFT_SHIFT = 340,
        LEFT_CONTROL = 341,
        LEFT_ALT = 342,
        LEFT_SUPER = 343,
        RIGHT_SHIFT = 344,
        RIGHT_CONTROL = 345,
        RIGHT_ALT = 346,
        RIGHT_SUPER = 347,
        MENU = 348
    }
    public sealed class InputManager
    {
        public const int LeftMouseButton = 0;
        public const int RightMouseButton = 1;
        public const int MiddleMouseButton = 2;
        internal InputManager(IntPtr address)
        {
            mAddress = address;
        }
        ~InputManager()
        {
            DestroyRef_Native(mAddress);
        }
        public bool MouseEnabled
        {
            get => GetMouseEnabled_Native(mAddress);
            set => SetMouseEnabled_Native(mAddress, value);
        }
        public Vector2 Mouse => GetMouse_Native(mAddress);
        public KeyState GetKey(Key key) => GetKey_Native(mAddress, key);
        public KeyState GetMouseButton(int button) => GetMouseButton_Native(mAddress, button);
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool GetMouseEnabled_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetMouseEnabled_Native(IntPtr address, bool value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector2 GetMouse_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern KeyState GetKey_Native(IntPtr address, Key key);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern KeyState GetMouseButton_Native(IntPtr address, int button);
    }
}