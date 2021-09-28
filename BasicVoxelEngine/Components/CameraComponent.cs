using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Components
{
    public sealed class CameraComponent
    {
        internal CameraComponent(IntPtr address)
        {
            mAddress = address;
        }
        public Vector3 Direction
        {
            get => GetDirection_Native(mAddress);
            set => SetDirection_Native(mAddress, value);
        }
        public Vector3 Up
        {
            get => GetUp_Native(mAddress);
            set => SetUp_Native(mAddress, value);
        }
        public bool Primary
        {
            get => GetPrimary_Native(mAddress);
            set => SetPrimary_Native(mAddress, value);
        }
        public float NearPlane
        {
            get => GetNearPlane_Native(mAddress);
            set => SetNearPlane_Native(mAddress, value);
        }
        public float FarPlane
        {
            get => GetFarPlane_Native(mAddress);
            set => SetFarPlane_Native(mAddress, value);
        }
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 GetDirection_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetDirection_Native(IntPtr address, Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector3 GetUp_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetUp_Native(IntPtr address, Vector3 value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool GetPrimary_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetPrimary_Native(IntPtr address, bool value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float GetNearPlane_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetNearPlane_Native(IntPtr address, float value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float GetFarPlane_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetFarPlane_Native(IntPtr address, float value);
    }
}