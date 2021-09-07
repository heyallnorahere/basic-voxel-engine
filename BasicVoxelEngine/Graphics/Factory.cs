using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Graphics
{
    public sealed class Factory
    {
        internal Factory(IntPtr address)
        {
            mNativeAddress = address;
        }
        ~Factory()
        {
            DestroyRef_Native(mNativeAddress);
        }
        public bool IsValid => mNativeAddress != IntPtr.Zero;
        internal IntPtr NativeAddress => mNativeAddress;
        private IntPtr mNativeAddress;
        public static implicit operator bool(Factory factory) => factory.IsValid;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
    }
}