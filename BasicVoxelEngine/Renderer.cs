using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine
{
    public sealed class Mesh
    {
        public unsafe void SetVertices<T>(IReadOnlyList<T> vertices) where T : unmanaged
        {
            FreeVertexBuffer();
            mVertexCount = vertices.Count;
            mVertexSize = sizeof(T);
            mVertexBuffer = AllocVertexBuffer_Native(mVertexSize * mVertexCount);
            for (int i = 0; i < mVertexCount; i++)
            {
                T vertex = vertices[i];
                CopyVertex_Native(mVertexBuffer, i, mVertexSize, &vertex);
            }
        }
        public void SetIndices(IReadOnlyList<uint> indices)
        {
            mIndices = new List<uint>(indices);
        }
        ~Mesh()
        {
            FreeVertexBuffer();
        }
        private void FreeVertexBuffer()
        {
            if (mVertexBuffer != IntPtr.Zero)
            {
                FreeVertexBuffer_Native(mVertexBuffer);
            }
        }
        private IntPtr mVertexBuffer = IntPtr.Zero;
        private int mVertexCount = 0;
        private int mVertexSize = 0;
        private IList<uint>? mIndices = null;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr AllocVertexBuffer_Native(int totalSize);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern unsafe void CopyVertex_Native(IntPtr buffer, int index, int stride, void* address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void FreeVertexBuffer_Native(IntPtr address);
    }
    public enum VertexAttributeType
    {
        Float,
        Int,
        Vector2,
        Vector2I,
        Vector3,
        Vector3I,
        Vector4,
        Vector4I,
        Matrix4
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct VertexAttribute
    {
        int Stride { get; set; }
        int Offset { get; set; }
        VertexAttributeType Type { get; set; }
        bool Normalize { get; set; }
    }
    public sealed class CommandList
    {
        internal CommandList(Renderer renderer)
        {
            mRenderer = renderer;
            mAddress = Create_Native(mRenderer.mAddress);
        }
        ~CommandList()
        {
            Destroy_Native(mAddress, mRenderer.mAddress);
        }
        public void AddMesh(Mesh mesh) => AddMesh_Native(mesh, mAddress, mRenderer.mAddress);
        public void Close(IReadOnlyList<VertexAttribute> vertexAttributes) => Close_Native(vertexAttributes, mAddress, mRenderer.mAddress);
        internal readonly IntPtr mAddress;
        internal readonly Renderer mRenderer;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr Create_Native(IntPtr rendererAddress);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Destroy_Native(IntPtr address, IntPtr rendererAddress);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AddMesh_Native(Mesh mesh, IntPtr address, IntPtr rendererAddress);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Close_Native(IReadOnlyList<VertexAttribute> vertexAttributes, IntPtr address, IntPtr rendererAddress);
    }
    public sealed class Renderer
    {
        internal Renderer(IntPtr address)
        {
            mAddress = address;
        }
        ~Renderer()
        {
            DestroyRef_Native(mAddress);
        }
        public CommandList CreateCommandList() => new CommandList(this);
        public void Render(CommandList commandList, Context context)
        {
            if (commandList.mRenderer.mAddress != mAddress)
            {
                throw new ArgumentException("The given command list was not created by this renderer!");
            }
            Render_Native(commandList.mAddress, context.mAddress, mAddress);
        }
        public void SetShader(Shader shader) => SetShader_Native(shader.mAddress, mAddress);
        public void SetTexture(int index, Texture texture) => SetTexture_Native(index, texture.mAddress, mAddress);
        internal readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Render_Native(IntPtr commandListAddress, IntPtr contextAddress, IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetShader_Native(IntPtr shaderAddress, IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetTexture_Native(int index, IntPtr textureAddress, IntPtr address);
    }
}
