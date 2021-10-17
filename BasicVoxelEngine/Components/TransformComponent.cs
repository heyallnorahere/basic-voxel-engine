using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Components
{
    public sealed class TransformComponent
    {
        internal TransformComponent(IntPtr address)
        {
            mAddress = address;
        }
        public Vector3 Translation
        {
            get
            {
                Vector3 translation;
                GetTranslation_Native(mAddress, out translation);
                return translation;
            }
            set => SetTranslation_Native(mAddress, value);
        }
        public Vector3 Rotation
        {
            get
            {
                Vector3 rotation;
                GetRotation_Native(mAddress, out rotation);
                return rotation;
            }
            set => SetRotation_Native(mAddress, value);
        }
        public Vector3 Scale
        {
            get
            {
                Vector3 scale;
                GetScale_Native(mAddress, out scale);
                return scale;
            }
            set => SetScale_Native(mAddress, value);
        }
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetTranslation_Native(IntPtr address, out Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetTranslation_Native(IntPtr address, Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetRotation_Native(IntPtr address, out Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetRotation_Native(IntPtr address, Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetScale_Native(IntPtr address, out Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetScale_Native(IntPtr address, Vector3 value);
    }
}