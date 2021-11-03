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
            mVertexSize = Marshal.SizeOf(typeof(T));
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
        Float = 1 << 0,
        Int = 1 << 1,
        Vector2 = 1 << 2,
        Vector2I = 1 << 3,
        Vector3 = 1 << 4,
        Vector3I = 1 << 5,
        Vector4 = 1 << 6,
        Vector4I = 1 << 7,
        Matrix4 = 1 << 8
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct VertexAttribute
    {
        public int Stride { get; set; }
        public int Offset { get; set; }
        public VertexAttributeType Type { get; set; }
        public bool Normalize { get; set; }
    }
    public sealed class CommandList
    {
        internal CommandList(Renderer renderer)
        {
            mAddress = Create_Native(renderer.mAddress, out mRendererAddress);
            mDestroyed = false;
        }
        public void Destroy()
        {
            if (!mDestroyed)
            {
                Destroy_Native(mAddress, mRendererAddress);
                mDestroyed = true;
            }
        }
        ~CommandList()
        {
            Destroy();
        }
        public void AddMesh(Mesh mesh)
        {
            if (mDestroyed)
            {
                throw new Exception("This CommandList has already been destroyed!");
            }
            AddMesh_Native(mesh, mAddress, mRendererAddress);
        }
        public void Close(List<VertexAttribute> vertexAttributes)
        {
            if (mDestroyed)
            {
                throw new Exception("This CommandList has already been destroyed!");
            }
            Close_Native(vertexAttributes, mAddress, mRendererAddress);
        }
        internal readonly IntPtr mAddress, mRendererAddress;
        private bool mDestroyed;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr Create_Native(IntPtr rendererAddress, out IntPtr refCopy);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Destroy_Native(IntPtr address, IntPtr rendererAddress);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AddMesh_Native(Mesh mesh, IntPtr address, IntPtr rendererAddress);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Close_Native(List<VertexAttribute> vertexAttributes, IntPtr address, IntPtr rendererAddress);
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
            if (!Helpers.AreRefsEqual(commandList.mRendererAddress, mAddress))
            {
                throw new ArgumentException("The given command list was not created by this renderer!");
            }
            Render_Native(commandList.mAddress, context.mAddress, mAddress);
        }
        public void SetShader(Shader? shader) => SetShader_Native(shader?.mAddress ?? IntPtr.Zero, mAddress);
        public void SetTexture(int index, Texture? texture) => SetTexture_Native(index, texture?.Address ?? IntPtr.Zero, mAddress);
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
