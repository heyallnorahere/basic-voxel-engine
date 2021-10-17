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
        public IReadOnlyList<Script> Scripts
        {
            get
            {
                var scripts = new List<Script>();
                int scriptCount = GetScriptCount_Native(mAddress);
                for (int i = 0; i < scriptCount; i++)
                {
                    scripts.Add(GetScript_Native(i, mAddress));
                }
                return scripts;
            }
        }
        public bool ScriptBound<T>() where T : Script
        {
            int scriptCount = GetScriptCount_Native(mAddress);
            for (int i = 0; i < scriptCount; i++)
            {
                var script = GetScript_Native(i, mAddress);
                if (script is T)
                {
                    return true;
                }
            }
            return false;
        }
        public T GetScript<T>() where T : Script
        {
            int scriptCount = GetScriptCount_Native(mAddress);
            for (int i = 0; i < scriptCount; i++)
            {
                var script = GetScript_Native(i, mAddress);
                if (script is T castScript)
                {
                    return castScript;
                }
            }
            throw new ArgumentException($"This script component does not contain a script of type: {typeof(T)}");
        }
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Script Bind_Native(Type type, IReadOnlyList<object?> args, IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetScriptCount_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Script GetScript_Native(int index, IntPtr address);
    }
}
