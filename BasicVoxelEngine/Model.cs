using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine
{
    public sealed class Model
    {
        public struct MeshData
        {
            public int VertexOffset { get; internal set; }
            public int IndexOffset { get; internal set; }
            public Dictionary<string, int> TextureOffsets { get; internal set; }
            // todo: textures
        }
        public struct Vertex
        {
            // todo: vectors lmao
        }
        public static Model LoadModel(string path, Factory factory)
        {
            if (!factory)
            {
                throw new ArgumentException("An invalid graphics factory was passed!");
            }
            IntPtr address = LoadModel_Native(path, factory.NativeAddress);
            return new Model(address);
        }
        internal Model(IntPtr nativeAddress)
        {
            mNativeAddress = nativeAddress;
        }
        ~Model()
        {
            DestroyRef_Native(mNativeAddress);
        }
        public int MeshCount => throw new NotImplementedException();
        public MeshData GetMesh() => throw new NotImplementedException();
        public List<Vertex> Vertices => throw new NotImplementedException();
        public List<uint> Indices => throw new NotImplementedException();
        private IntPtr mNativeAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr LoadModel_Native(string path, IntPtr factory);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
    }
}