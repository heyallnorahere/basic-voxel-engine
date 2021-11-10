using BasicVoxelEngine.Graphics;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine
{
    public sealed class Buffer
    {
        static Buffer()
        {
            mDirectCopyTypes = new List<Type>
            {
                typeof(int),
                typeof(uint),
                typeof(float),
                typeof(double),
                typeof(bool),
                typeof(Vector2),
                typeof(Vector2I),
                typeof(Vector3),
                typeof(Vector3I)
            };
        }
        public Buffer()
        {
            mOwned = true;
            Create_Native(out mAddress);
        }
        internal Buffer(IntPtr address)
        {
            mOwned = false;
            mAddress = address;
        }
        ~Buffer()
        {
            if (mOwned)
            {
                Destroy_Native(mAddress);
            }
        }
        public void Alloc(int size) => Alloc_Native(mAddress, size);
        public void Free() => Free_Native(mAddress);
        public void Zero() => Zero_Native(mAddress);
        public unsafe void Copy(void* data, int size, int offset = 0) => Copy_Native(mAddress, data, size, offset);
        public unsafe void Copy<T>(T data, int offset = 0) where T : unmanaged => Copy(&data, sizeof(T), offset);
        public void Copy(Buffer buffer, int offset = 0) => Copy_Buffer_Native(mAddress, buffer.mAddress, offset);
        // i should not do this
        private class Copier<T> where T : unmanaged
        {
            public Copier(Buffer buffer)
            {
                mBuffer = buffer;
            }
            public void Exec(T data, int offset)
            {
                mBuffer.Copy(data, offset);
            }
            private readonly Buffer mBuffer;
        }
        public void CopyToGPUBuffer(object data, ShaderReflectionType shaderType, int offset = 0)
        {
            Type type = data.GetType();
            if (mDirectCopyTypes.Contains(type))
            {
                var copierType = typeof(Copier<>).MakeGenericType(type);
                var constructor = copierType.GetConstructor(new Type[] { typeof(Buffer) });
                var copier = constructor?.Invoke(new object[] { this });
                var execMethod = copierType.GetMethod("Exec"); // fsr "nameof" doesnt work
                execMethod?.Invoke(copier, new object[] { this, offset });
                return;
            }
            foreach (var cpuField in type.GetFields())
            {
                if (!shaderType.Fields.ContainsKey(cpuField.Name))
                {
                    continue;
                }
                var fieldData = cpuField.GetValue(data);
                if (fieldData == null)
                {
                    continue;
                }
                Type cpuFieldType = cpuField.FieldType;
                var gpuField = shaderType.Fields[cpuField.Name];
                var gpuFieldType = gpuField.Type;
                int fieldOffset = offset + gpuField.Offset;
                if (cpuFieldType.Implements(typeof(IList)))
                {
                    var list = (IList)fieldData;
                    int arraySize = list.Count > gpuFieldType.ArraySize ? gpuFieldType.ArraySize : list.Count;
                    for (int i = 0; i < arraySize; i++)
                    {
                        if (list[i] == null)
                        {
                            continue;
                        }
                        int copyOffset = i * gpuFieldType.ArrayStride + fieldOffset;
                        CopyToGPUBuffer(list[i], gpuFieldType, copyOffset);
                    }
                }
                else
                {
                    CopyToGPUBuffer(fieldData, gpuFieldType, fieldOffset);
                }
            }
        }
        public int Size => GetSize_Native(mAddress);
        public static implicit operator bool(Buffer buffer) => IsAllocated_Native(buffer.mAddress);
        private readonly bool mOwned;
        internal readonly IntPtr mAddress;
        private static readonly List<Type> mDirectCopyTypes;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Create_Native(out IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Destroy_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Alloc_Native(IntPtr address, int size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Free_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Zero_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static unsafe extern void Copy_Native(IntPtr address, void* data, int size, int offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Copy_Buffer_Native(IntPtr address, IntPtr buffer, int offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetSize_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsAllocated_Native(IntPtr address);
    }
}
