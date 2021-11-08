using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Graphics
{
    public sealed class ComputePipeline
    {
        internal ComputePipeline(IntPtr address)
        {
            mAddress = address;
        }
        ~ComputePipeline()
        {
            DestroyRef_Native(mAddress);
        }
        public void BindUniformBuffer(UniformBuffer uniformBuffer) => BindUniformBuffer_Native(mAddress, uniformBuffer.mAddress);
        public void BindStorageBuffer(StorageBuffer storageBuffer) => BindStorageBuffer_Native(mAddress, storageBuffer.mAddress);
        public void Dispatch(uint groupCountX = 1, uint groupCountY = 1, uint groupCountZ = 1) => Dispatch_Native(mAddress, groupCountX, groupCountY, groupCountZ);
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void BindUniformBuffer_Native(IntPtr address, IntPtr uniformBuffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void BindStorageBuffer_Native(IntPtr address, IntPtr storageBuffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Dispatch_Native(IntPtr address, uint groupCountX, uint groupCountY, uint groupCountZ);
    }
}
