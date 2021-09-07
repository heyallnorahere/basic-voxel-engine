using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Lighting
{
    public sealed class PointLight : Light
    {
        public PointLight() : this(Create_Native()) { }
        internal PointLight(IntPtr address) : base(address) { }
        public float Constant
        {
            set => SetConstant_Native(mAddress, value);
        }
        public float Linear
        {
            set => SetLinear_Native(mAddress, value);
        }
        public float Quadratic
        {
            set => SetQuadratic_Native(mAddress, value);
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr Create_Native();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetConstant_Native(IntPtr address, float constant);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetLinear_Native(IntPtr address, float linear);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetQuadratic_Native(IntPtr address, float quadratic);
    }
}