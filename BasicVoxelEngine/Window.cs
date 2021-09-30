using BasicVoxelEngine.Graphics;
using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public sealed class Window
    {
        internal Window(IntPtr address)
        {
            mAddress = address;
        }
        ~Window()
        {
            DestroyRef_Native(mAddress);
        }
        public Context Context => new Context(GetContext_Native(mAddress));
        public Vector2I FramebufferSize => GetFramebufferSize_Native(mAddress);
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetContext_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector2I GetFramebufferSize_Native(IntPtr address);
    }
}