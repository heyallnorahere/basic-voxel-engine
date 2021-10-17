using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Graphics
{
    public sealed class Context
    {
        internal Context(IntPtr address)
        {
            mAddress = address;
        }
        ~Context()
        {
            DestroyRef_Native(mAddress);
        }
        public void MakeCurrent() => MakeCurrent_Native(mAddress);
        public void DrawIndexed(int indexCount) => DrawIndexed_Native(mAddress, indexCount);
        internal readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void MakeCurrent_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DrawIndexed_Native(IntPtr address, int indexCount);
    }
}