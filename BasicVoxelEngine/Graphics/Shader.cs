using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Graphics
{
    internal sealed class UniformReturnValue
    {
        public object? Data { get; set; }
    }
    public sealed class Shader
    {
        static Shader()
        {
            InitializeUniforms_Native();
        }
        internal Shader(IntPtr address)
        {
            mAddress = address;
        }
        ~Shader()
        {
            DestroyRef_Native(mAddress);
        }
        public void Reload() => Reload_Native(mAddress);
        public void Bind() => Bind_Native(mAddress);
        public void Unbind() => Unbind_Native(mAddress);
        public void Set<T>(string name, T value) where T : notnull => Set_Native(mAddress, name, value, typeof(T));
        public T? Get<T>(string name) where T : notnull
        {
            var returnValue = new UniformReturnValue();
            Get_Native(mAddress, name, returnValue, typeof(T));
            return (T?)returnValue.Data;
        }
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Reload_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Bind_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Unbind_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Set_Native(IntPtr address, string name, object value, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Get_Native(IntPtr address, string name, UniformReturnValue value, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void InitializeUniforms_Native();
    }
}