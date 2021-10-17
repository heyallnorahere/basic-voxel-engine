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
            get => GetTranslation_Native(mAddress);
            set => SetTranslation_Native(mAddress, value);
        }
        public Vector3 Rotation
        {
            get => GetRotation_Native(mAddress);
            set => SetRotation_Native(mAddress, value);
        }
        public Vector3 Scale
        {
            get => GetScale_Native(mAddress);
            set => SetScale_Native(mAddress, value);
        }
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 GetTranslation_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetTranslation_Native(IntPtr address, Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 GetRotation_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetRotation_Native(IntPtr address, Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 GetScale_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetScale_Native(IntPtr address, Vector3 value);
    }
}