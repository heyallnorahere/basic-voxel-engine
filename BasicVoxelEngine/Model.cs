using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine
{
    internal sealed class MeshTextureOffsetData
    {
        public MeshTextureOffsetData(IntPtr address)
        {
            mAddress = address;
        }
        ~MeshTextureOffsetData()
        {
            Destroy_Native(mAddress);
        }
        public IReadOnlyDictionary<string, int> Get()
        {
            var dict = new Dictionary<string, int>();
            int count = GetCount_Native(mAddress);
            for (int i = 0; i < count; i++)
            {
                string key = GetKey_Native(mAddress, i);
                int value = GetValue_Native(mAddress, key);
                dict.Add(key, value);
            }
            return dict;
        }
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Destroy_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetCount_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string GetKey_Native(IntPtr address, int index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetValue_Native(IntPtr address, string key);
    }
    public struct MeshData
    {
        public int VertexOffset { get; internal set; }
        public int IndexOffset { get; internal set; }
        public IReadOnlyDictionary<string, int> TextureOffsets { get; internal set; }
        public IReadOnlyList<Texture> Textures { get; internal set; }
    }
    public sealed class Model
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct Vertex
        {
            public Vector3 Position { get; set; }
            public Vector3 Normal { get; set; }
            public Vector2 UV { get; set; }
            public Vector3 Tangent { get; set; }
            public Vector3 Bitangent { get; set; }
        }
        public static Model LoadModel(string path, Factory factory)
        {
            if (factory.mNativeAddress == IntPtr.Zero)
            {
                throw new ArgumentException("An invalid graphics factory was passed!");
            }
            IntPtr address = LoadModel_Native(path, factory.mNativeAddress);
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
        public int MeshCount => GetMeshCount_Native(mNativeAddress);
        public MeshData GetMesh(int index)
        {
            int vertexOffset, indexOffset, textureCount;
            var textureOffsetData = GetMesh_Native(mNativeAddress, index, out vertexOffset, out indexOffset, out textureCount);
            var textures = new List<Texture>();
            for (int i = 0; i < textureCount; i++)
            {
                IntPtr address = GetTexture_Native(mNativeAddress, index, i);
                textures.Add(new Texture(address));
            }
            return new MeshData
            {
                VertexOffset = vertexOffset,
                IndexOffset = indexOffset,
                TextureOffsets = textureOffsetData.Get(),
                Textures = textures
            };
        }
        public IReadOnlyList<Vertex> Vertices
        {
            get
            {
                int vertexCount = GetVertexCount_Native(mNativeAddress);
                var vertices = new List<Vertex>();
                for (int i = 0; i < vertexCount; i++)
                {
                    vertices.Add(GetVertex_Native(mNativeAddress, i));
                }
                return vertices;
            }
        }
        public IReadOnlyList<uint> Indices
        {
            get
            {
                int indexCount = GetIndexCount_Native(mNativeAddress);
                var indices = new List<uint>();
                for (int i = 0; i < indexCount; i++)
                {
                    indices.Add(GetIndex_Native(mNativeAddress, i));
                }
                return indices;
            }
        }
        internal readonly IntPtr mNativeAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr LoadModel_Native(string path, IntPtr factory);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetMeshCount_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern MeshTextureOffsetData GetMesh_Native(IntPtr address, int index, out int vertexOffset, out int indexOffset, out int textureCount);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetTexture_Native(IntPtr address, int meshIndex, int textureIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetVertexCount_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vertex GetVertex_Native(IntPtr address, int index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetIndexCount_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern uint GetIndex_Native(IntPtr address, int index);
    }
}