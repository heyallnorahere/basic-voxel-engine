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
        public Texture CreateTexture(ImageData imageData) => new Texture(CreateTexture_Native(mNativeAddress, imageData));
        public Texture CreateTexture(string path) => CreateTexture(ImageData.Load(path));
        internal readonly IntPtr mNativeAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr CreateTexture_Native(IntPtr address, ImageData imageData);
    }
}