using BasicVoxelEngine.Graphics;
using BasicVoxelEngine.Lighting;
using System;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public class Block : RegisteredObject<Block>
    {
        public Block() { }
        // todo: pass and object_factory instance for model loading
        /// <summary>
        /// This function is called just before the game runs and after every other block is registered.
        /// </summary>
        protected virtual void Load(Factory factory, NamespacedName namespacedName)
        {
            Logger.Print(Logger.Severity.Info, "base has been hit");
        }
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
        /// <summary>
        /// The model that represents this image. Default is null.
        /// Use <see cref="Model.LoadModel(string, Factory)" /> to load a model.
        /// </summary>
        public virtual Model? Model
        {
            get
            {
                if (IsNative)
                {
                    IntPtr address = GetModel_Native(NativeAddress);
                    if (address != IntPtr.Zero)
                    {
                        return new Model(address);
                    }
                }
                return null;
            }
        }
        public virtual Light? Light
        {
            get
            {
                if (IsNative)
                {
                    LightType lightType;
                    IntPtr address = GetLight_Native(NativeAddress, out lightType);
                    if (address != IntPtr.Zero)
                    {
                        switch (lightType)
                        {
                            case LightType.Spotlight:
                                return new Spotlight(address);
                            case LightType.PointLight:
                                return new PointLight(address);
                            default:
                                throw new NotImplementedException();
                        }
                    }
                }
                return null;
            }
        }
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
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetModel_Native(IntPtr nativeAddress);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetLight_Native(IntPtr nativeAddress, out LightType type);
    }
}