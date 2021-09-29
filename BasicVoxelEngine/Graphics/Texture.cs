using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Graphics
{
    public class ImageData
    {
        public static ImageData Load(string path)
        {
            IntPtr address = Load_Native(path);
            int width = GetWidth_Native(address);
            int height = GetHeight_Native(address);
            int channels = GetChannels_Native(address);
            var data = new List<byte>();
            for (int i = 0; i < width * height * channels; i++)
            {
                data.Add(GetByte_Native(address, i));
            }
            Destroy_Native(address);
            return new ImageData(data, width, height, channels);
        }
        public ImageData(IReadOnlyList<byte> data, int width, int height, int channels)
        {
            Width = width;
            Height = height;
            Channels = channels;
            Data = new List<byte>(data);
        }
        public int Width { get; }
        public int Height { get; }
        public int Channels { get; }
        public IReadOnlyList<byte> Data { get; }
        ///<summary>
        /// For native code
        ///</summary>
        internal byte GetByte(int index) => Data[index];
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr Load_Native(string path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Destroy_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetWidth_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetHeight_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetChannels_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern byte GetByte_Native(IntPtr address, int index);
    }
    public sealed class Texture
    {
        internal Texture(IntPtr address)
        {
            mAddress = address;
        }
        ~Texture()
        {
            DestroyRef_Native(mAddress);
        }
        public Vector2I Size => GetSize_Native(mAddress);
        public int Channels => GetChannels_Native(mAddress);
        private readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Vector2I GetSize_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetChannels_Native(IntPtr address);
    }
}