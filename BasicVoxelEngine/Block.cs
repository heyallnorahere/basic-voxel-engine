using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public class Block : RegisteredObject<Block>
    {
        public Block() { }
        /// <summary>
        /// The opacity of this block. Default is 100% (1.0).
        /// </summary>
        public virtual float Opacity
        {
            get
            {
                if (IsNative)
                {
                    return GetOpacity_Native(NativeAddress);
                }
                else
                {
                    return 1f;
                }
            }
        }
        /// <summary>
        /// If this block is solid or not. Default is true.
        /// </summary>
        public virtual bool Solid
        {
            get
            {
                if (IsNative)
                {
                    return GetSolid_Native(NativeAddress);
                }
                else
                {
                    return true;
                }
            }
        }
        // todo: Model and Light
        /// <summary>
        /// The friendly name of this block.
        /// This property MUST be overwritten for managed <see cref="Block" /> definitions.
        /// </summary>
        public virtual string FriendlyName => GetFriendlyName_Native(NativeAddress);
        private bool IsNative => base.NativeAddress != null;
        private new IntPtr NativeAddress => base.NativeAddress ?? throw new NullReferenceException();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float GetOpacity_Native(IntPtr nativeAddress);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool GetSolid_Native(IntPtr nativeAddress);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string GetFriendlyName_Native(IntPtr nativeAddress);
    }
}