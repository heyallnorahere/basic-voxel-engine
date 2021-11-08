using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public sealed class Buffer
    {
        public Buffer()
        {
            mOwned = true;
            Create_Native(out mAddress);
        }
        internal Buffer(IntPtr address)
        {
            mOwned = false;
            mAddress = address;
        }
        ~Buffer()
        {
            if (mOwned)
            {
                Destroy_Native(mAddress);
            }
        }
        public void Alloc(int size) => Alloc_Native(mAddress, size);
        public void Free() => Free_Native(mAddress);
        public void Zero() => Zero_Native(mAddress);
        public unsafe void Copy(void* data, int size, int offset = 0) => Copy_Native(mAddress, data, size, offset);
        public unsafe void Copy<T>(T data, int offset = 0) where T : unmanaged => Copy(&data, sizeof(T), offset);
        public void Copy(Buffer buffer, int offset = 0) => Copy_Buffer_Native(mAddress, buffer.mAddress, offset);
        public int Size => GetSize_Native(mAddress);
        public static implicit operator bool(Buffer buffer) => IsAllocated_Native(buffer.mAddress);
        private readonly bool mOwned;
        internal readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Create_Native(out IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Destroy_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Alloc_Native(IntPtr address, int size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Free_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Zero_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static unsafe extern void Copy_Native(IntPtr address, void* data, int size, int offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Copy_Buffer_Native(IntPtr address, IntPtr buffer, int offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetSize_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsAllocated_Native(IntPtr address);
    }
}
