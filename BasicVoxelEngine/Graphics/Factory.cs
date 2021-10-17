using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Graphics
{
    public enum GraphicsAPI
    {
        OPENGL = 1 << 0,
        VULKAN = 1 << 1
    }
    internal sealed class ShaderSourceList
    {
        public ShaderSourceList(IEnumerable<string> paths)
        {
            var pathsList = new List<string>();
            foreach (string path in paths)
            {
                pathsList.Add(path);
            }
            mPaths = pathsList;
        }
        public int Count => mPaths.Count;
        public string GetPath(int index) => mPaths[index];
        private readonly IReadOnlyList<string> mPaths;
    }
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
        public Context CreateContext() => new Context(CreateContext_Native(mNativeAddress));
        public Shader CreateShader(IEnumerable<string> paths) => new Shader(CreateShader_Native(mNativeAddress, new ShaderSourceList(paths)));
        public GraphicsAPI API => GetGraphicsAPI_Native(mNativeAddress);
        internal readonly IntPtr mNativeAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr CreateTexture_Native(IntPtr address, ImageData imageData);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr CreateContext_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr CreateShader_Native(IntPtr address, ShaderSourceList sourceList);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern GraphicsAPI GetGraphicsAPI_Native(IntPtr address);
    }
}