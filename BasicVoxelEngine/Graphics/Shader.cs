using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace BasicVoxelEngine.Graphics
{
    internal sealed class UniformReturnValue
    {
        public object? Data { get; set; }
    }
    public sealed class ShaderReflectionField
    {
        internal ShaderReflectionField(IntPtr fieldData, ShaderReflectionData reflectionData)
        {
            Offset = GetOffset(fieldData);
            IntPtr typeData = GetType(fieldData);
            Type = ShaderReflectionType.GetReflectionType(typeData, reflectionData);
        }
        public int Offset { get; }
        public ShaderReflectionType Type { get; }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetOffset(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetType(IntPtr address);
    }
    public sealed class ShaderReflectionType
    {
        internal static ShaderReflectionType GetReflectionType(IntPtr typeData, ShaderReflectionData reflectionData)
        {
            if (reflectionData.mTypes.ContainsKey(typeData))
            {
                return reflectionData.mTypes[typeData];
            }
            else
            {
                var type = new ShaderReflectionType(typeData, reflectionData);
                reflectionData.mTypes.Add(typeData, type);
                return type;
            }
        }
        private ShaderReflectionType(IntPtr typeData, ShaderReflectionData reflectionData)
        {
            Name = GetName(typeData);
            Size = GetSize(typeData);
            ArrayStride = GetArrayStride(typeData);
            ArraySize = GetArraySize(typeData);
            var fields = new Dictionary<string, ShaderReflectionField>();
            var names = new List<string>();
            GetFieldNames(typeData, names);
            foreach (string name in names)
            {
                IntPtr fieldData = GetField(typeData, name);
                var field = new ShaderReflectionField(fieldData, reflectionData);
                fields.Add(name, field);
            }
            Fields = fields;
        }
        public int FindOffset(string fieldName)
        {
            string name = fieldName;
            string? subname = null;
            var segments = fieldName.Split('.');
            if (segments.Length >= 2)
            {
                name = segments[0];
                subname = segments[1];
                for (int i = 2; i < segments.Length; i++)
                {
                    subname += "." + segments[i];
                }
            }
            int index = -1;
            int openBracket = name.IndexOf('[');
            if (openBracket != -1)
            {
                int closeBracket = name.IndexOf(']');
                if (closeBracket <= openBracket + 1 || closeBracket < name.Length - 1)
                {
                    throw new ArgumentException("Invalid index operator call!");
                }
                int indexStart = openBracket + 1;
                var indexString = name.Substring(indexStart, closeBracket - indexStart);
                name = name.Substring(0, openBracket);
                index = int.Parse(indexString);
            }
            if (!Fields.ContainsKey(name))
            {
                throw new ArgumentException($"{name} is not a field in this structure!");
            }
            var field = Fields[name];
            if (index != -1 && field.Type.ArrayStride == 0)
            {
                throw new ArgumentException("Attempted to index into a non-array field!");
            }
            if (index >= field.Type.ArraySize)
            {
                throw new IndexOutOfRangeException();
            }
            if (index < 0)
                index = 0;
            int offset = field.Offset + (index * field.Type.ArrayStride);
            if (subname != null)
            {
                return offset + field.Type.FindOffset(subname);
            }
            else
            {
                return offset;
            }
        }
        public string Name { get; }
        public int Size { get; }
        public int ArrayStride { get; }
        public int ArraySize { get; }
        public IReadOnlyDictionary<string, ShaderReflectionField> Fields { get; }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string GetName(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetSize(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetArrayStride(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetArraySize(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetFieldNames(IntPtr address, List<string> names);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetField(IntPtr address, string name);
    }
    public sealed class ShaderResourceData
    {
        internal ShaderResourceData(IntPtr resourceData, ShaderReflectionData reflectionData)
        {
            Name = GetName(resourceData);
            IntPtr typeData = GetType(resourceData);
            Type = ShaderReflectionType.GetReflectionType(typeData, reflectionData);
            // ill do shader stage later
        }
        public string Name { get; }
        public ShaderReflectionType Type { get; }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string GetName(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetType(IntPtr address);
    }
    public sealed class ShaderDescriptorSetData
    {
        internal ShaderDescriptorSetData(IntPtr descriptorSetData, ShaderReflectionData reflectionData)
        {
            UniformBuffers = ExtractResources(descriptorSetData, reflectionData, GetUniformBufferIndices, GetUniformBufferData);
            SampledImages = ExtractResources(descriptorSetData, reflectionData, GetSampledImageIndices, GetSampledImage);
            StorageBuffers = ExtractResources(descriptorSetData, reflectionData, GetStorageBufferIndices, GetStorageBufferData);
            PushConstantBuffers = ExtractResources(descriptorSetData, reflectionData, GetPushConstantBufferIndices, GetPushConstantBufferData);
        }
        private delegate void GetIndices(IntPtr address, List<uint> indices);
        private delegate IntPtr GetData(IntPtr address, uint index);
        private IReadOnlyDictionary<uint, ShaderResourceData> ExtractResources(IntPtr setData, ShaderReflectionData reflectionData, GetIndices getIndices, GetData getData)
        {
            var indices = new List<uint>();
            var resources = new Dictionary<uint, ShaderResourceData>();
            getIndices(setData, indices);
            foreach (uint index in indices)
            {
                IntPtr resourceData = getData(setData, index);
                var resource = new ShaderResourceData(resourceData, reflectionData);
                resources.Add(index, resource);
            }
            return resources;
        }
        public IReadOnlyDictionary<uint, ShaderResourceData> UniformBuffers { get; }
        public IReadOnlyDictionary<uint, ShaderResourceData> SampledImages { get; }
        public IReadOnlyDictionary<uint, ShaderResourceData> StorageBuffers { get; }
        public IReadOnlyDictionary<uint, ShaderResourceData> PushConstantBuffers { get; }
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetUniformBufferIndices(IntPtr address, List<uint> indices);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetUniformBufferData(IntPtr address, uint index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetSampledImageIndices(IntPtr address, List<uint> indices);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetSampledImage(IntPtr address, uint index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetStorageBufferIndices(IntPtr address, List<uint> indices);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetStorageBufferData(IntPtr address, uint index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetPushConstantBufferIndices(IntPtr address, List<uint> indices);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetPushConstantBufferData(IntPtr address, uint index);
    }
    public sealed class ShaderReflectionData
    {
        internal ShaderReflectionData(IntPtr address)
        {
            mTypes = new Dictionary<IntPtr, ShaderReflectionType>();
            var descriptorSets = new Dictionary<uint, ShaderDescriptorSetData>();
            var indices = new List<uint>();
            GetDescriptorSetIndices(address, indices);
            foreach (uint index in indices)
            {
                IntPtr descriptorSetData = GetDescriptorSetData(address, index);
                var data = new ShaderDescriptorSetData(descriptorSetData, this);
                descriptorSets.Add(index, data);
            }
            DescriptorSets = descriptorSets;
        }
        private bool TryFindResource(IReadOnlyDictionary<uint, ShaderResourceData> resourceGroup, string name, ref uint binding)
        {
            foreach (uint currentBinding in resourceGroup.Keys)
            {
                var resourceName = resourceGroup[currentBinding].Name;
                if (resourceName == name)
                {
                    binding = currentBinding;
                    return true;
                }
            }
            return false;
        }
        public bool TryFindResource(string name, ref uint set, ref uint binding)
        {
            foreach (uint currentSet in DescriptorSets.Keys)
            {
                set = currentSet;
                var setData = DescriptorSets[currentSet];
                if (TryFindResource(setData.UniformBuffers, name, ref binding)) return true;
                if (TryFindResource(setData.StorageBuffers, name, ref binding)) return true;
                if (TryFindResource(setData.SampledImages, name, ref binding)) return true;
                if (TryFindResource(setData.PushConstantBuffers, name, ref binding)) return true;
            }
            return false;
        }
        public void FindResource(string name, out uint set, out uint binding)
        {
            set = 0;
            binding = 0;
            if (!TryFindResource(name, ref set, ref binding))
            {
                throw new ArgumentException("Could not find the requested resource!");
            }
        }
        public IReadOnlyDictionary<uint, ShaderDescriptorSetData> DescriptorSets { get; }
        public IReadOnlyList<ShaderReflectionType> Types
        {
            get
            {
                var types = new List<ShaderReflectionType>();
                foreach (var type in mTypes.Values)
                {
                    types.Add(type);
                }
                return types;
            }
        }
        internal readonly Dictionary<IntPtr, ShaderReflectionType> mTypes;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetDescriptorSetIndices(IntPtr address, List<uint> indices);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetDescriptorSetData(IntPtr address, uint index);
    }
    public sealed class Shader
    {
        static Shader()
        {
            InitializeUniforms_Native();
        }
        internal Shader(IntPtr address)
        {
            mAddress = address;
        }
        ~Shader()
        {
            DestroyRef_Native(mAddress);
        }
        public void Reload() => Reload_Native(mAddress);
        public void Bind() => Bind_Native(mAddress);
        public void Unbind() => Unbind_Native(mAddress);
        public void Set<T>(string name, T value) where T : struct => Set_Native(mAddress, name, value, typeof(T));
        public T? Get<T>(string name) where T : struct
        {
            var returnValue = new UniformReturnValue();
            Get_Native(mAddress, name, returnValue, typeof(T));
            return (T?)returnValue.Data;
        }
        public ShaderReflectionData ReflectionData => new ShaderReflectionData(GetReflectionData_Native(mAddress));
        internal readonly IntPtr mAddress;
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void DestroyRef_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Reload_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Bind_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Unbind_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Set_Native(IntPtr address, string name, object value, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Get_Native(IntPtr address, string name, UniformReturnValue value, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetReflectionData_Native(IntPtr address);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void InitializeUniforms_Native();
    }
}