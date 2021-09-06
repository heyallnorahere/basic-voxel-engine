using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Lighting
{
    public sealed class Spotlight : Light
    {
        public static Spotlight Create()
        {
            return new Spotlight(Create_Native());
        }
        internal Spotlight(IntPtr address) : base(address) { }
        public Vector3 Direction
        {
            set
            {
                SetDirection_Native(mAddress, value);
            }
        }
        public float Cutoff
        {
            set
            {
                SetCutoff_Native(mAddress, value);
            }
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr Create_Native();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetDirection_Native(IntPtr address, Vector3 direction);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetCutoff_Native(IntPtr address, float cutoff);
    }
}