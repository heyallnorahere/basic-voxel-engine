using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Lighting
{
    public enum LightType
    {
        Spotlight = (1 << 0),
        PointLight = (1 << 1),
        DirectionalLight = (1 << 2)
    }
    public abstract class Light
    {
        internal Light(IntPtr address)
        {
            mAddress = address;
        }
        ~Light()
        {
            Destroy_Native(mAddress);
        }
        public Vector3 Color
        {
            set
            {
                SetColor_Native(mAddress, value);
            }
        }
        public float AmbientStrength
        {
            set
            {
                SetAmbientStrength_Native(mAddress, value);
            }
        }
        public float SpecularStrength
        {
            set
            {
                SetSpecularStrength_Native(mAddress, value);
            }
        }
        public LightType Type => GetType_Native(mAddress);
        protected readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Destroy_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetColor_Native(IntPtr address, Vector3 color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetAmbientStrength_Native(IntPtr address, float strength);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetSpecularStrength_Native(IntPtr address, float strength);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern LightType GetType_Native(IntPtr address);
    }
}