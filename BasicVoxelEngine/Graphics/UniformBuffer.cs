using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Graphics
{
    public sealed class UniformBuffer
    {
        internal UniformBuffer(IntPtr address)
        {
            mAddress = address;
        }
        ~UniformBuffer()
        {
            DestroyRef_Native(mAddress);
        }
        public unsafe void SetData(void* data, int size, int offset = 0) => SetData_Native(mAddress, data, size, offset);
        public unsafe void SetData<T>(T data, int offset = 0) where T : unmanaged => SetData(&data, sizeof(T), offset);
        public void SetData(Buffer buffer, int offset = 0) => SetData_Buffer_Native(mAddress, buffer.mAddress, offset);
        public void Activate() => Activate_Native(mAddress);
        public int Size => GetSize_Native(mAddress);
        public uint Binding => GetBinding_Native(mAddress);
        internal readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static unsafe extern void SetData_Native(IntPtr address, void* data, int size, int offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetData_Buffer_Native(IntPtr address, IntPtr buffer, int offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Activate_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetSize_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern uint GetBinding_Native(IntPtr address);
    }
}
