using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Components
{
    public sealed class ScriptComponent
    {
        internal ScriptComponent(IntPtr address)
        {
            mAddress = address;
        }
        public T Bind<T>(params object?[] args) where T : Script => (T)Bind_Native(typeof(T), new List<object?>(args), mAddress);
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object Bind_Native(Type type, IReadOnlyList<object?> args, IntPtr address);
    }
}
