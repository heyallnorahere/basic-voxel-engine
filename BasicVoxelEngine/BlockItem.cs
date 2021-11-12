using BasicVoxelEngine.Components;
using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;
using System.IO;

namespace BasicVoxelEngine
{
    public abstract class BlockItem : Item
    {
        public BlockItem(Type blockType)
        {
            mBlockType = blockType;
            if (!mBlockType.DerivesFrom(typeof(Block)))
            {
                throw new ArgumentException("The given type does not derive from Block!");
            }
            mBlock = null;
            mFactory = null;
            mImage = null;
        }
        protected override void Load(Factory factory, NamespacedName namespacedName)
        {
            mFactory = factory;
            if (mGetFaceDirection == null)
            {
                mGetFaceDirection = mFactory.CreateShader(new string[] { AssetManager.GetAssetPath("shaders:compute:get_face_direction.glsl") });
            }
            var blockRegister = Registry.GetRegister<Block>();
            foreach (Block block in blockRegister)
            {
                if (block.GetType() == mBlockType)
                {
                    mBlock = block;
                    break;
                }
            }
            if (mBlock == null)
            {
                throw new ArgumentException("The given block type is not registered!");
            }
        }
        public override string FriendlyName => Block.FriendlyName;
        public override Texture? Image
        {
            get
            {
                if (mImage == null && mFactory != null)
                {
                    // todo: render block in a framebuffer
                    var blockRegister = Registry.GetRegister<Block>();
                    NamespacedName namespacedName = blockRegister.GetNamespacedName(mBlockType);
                    mImage = mFactory.CreateTexture(AssetManager.GetAssetPath($"block:{namespacedName}.png"));
                }
                return mImage;
            }
        }
        public override Action<ClickActionArgs>? RightClick => PlaceBlock;
        private struct PlayerData
        {
            public Vector3 position, direction;
        }
        private struct Vertex
        {
            public Vector3 position, normal;
        }
        private struct ModelData
        {
            public int vertexCount, indexCount;
            public Vertex[] vertices;
            public uint[] indices;
        }
        private struct BlockData
        {
            public Vector3I position;
            public bool hasModel;
            public ModelData? model;
        }
        private void PlaceBlock(ClickActionArgs args)
        {
            if (mFactory == null || mGetFaceDirection == null)
            {
                throw new Exception($"{FriendlyName} was not loaded correctly!");
            }
            Entity user = args.User;
            var cameraComponent = user.GetComponent<CameraComponent>();
            var transformComponent = user.GetComponent<TransformComponent>();
            var position = transformComponent.Translation;
            var direction = cameraComponent.Direction;
            var world = Application.World;
            var candidates = new HashSet<Vector3I>();
            const float rayRadius = 0.5f;
            var rayOffsets = new Vector3[] {
                new Vector3(rayRadius, 0f, 0f),
                new Vector3(0f, rayRadius, 0f),
                new Vector3(0f, 0f, rayRadius),
            };
            for (var offset = new Vector3(0f); offset.Length < args.Reach; offset += direction)
            {
                foreach (Vector3 rayOffset in rayOffsets)
                {
                    var worldPosition = position + offset + rayOffset;
                    var blockPosition = new Vector3I
                    {
                        X = (int)worldPosition.X,
                        Y = (int)worldPosition.Y,
                        Z = (int)worldPosition.Z
                    };
                    int blockType;
                    world.GetBlock(blockPosition, out blockType);
                    if (blockType != 0)
                    {
                        // HashSet<> wont add entries that it already contains
                        candidates.Add(blockPosition);
                    }
                }
            }
            if (candidates.Count <= 0)
            {
                return;
            }
            var computePipeline = mFactory.CreateComputePipeline(mGetFaceDirection);
            var reflectionData = mGetFaceDirection.ReflectionData;
            Buffer playerBufferData, blockBufferData, outputBufferData;
            uint set, binding;
            reflectionData.FindResource("playerData", out set, out binding);
            var playerBuffer = mFactory.CreateUniformBuffer(out playerBufferData, reflectionData, binding, set);
            var playerBufferType = reflectionData.DescriptorSets[set].UniformBuffers[binding].Type;
            reflectionData.FindResource("blockData", out set, out binding);
            var blockBuffer = mFactory.CreateUniformBuffer(out blockBufferData, reflectionData, binding, set);
            var blockBufferType = reflectionData.DescriptorSets[set].UniformBuffers[binding].Type;
            reflectionData.FindResource("outputData", out set, out binding);
            var outputBuffer = mFactory.CreateStorageBuffer(out outputBufferData, reflectionData, binding, set);
            var outputBufferType = reflectionData.DescriptorSets[set].StorageBuffers[binding].Type;
            computePipeline.BindUniformBuffer(playerBuffer);
            computePipeline.BindUniformBuffer(blockBuffer);
            computePipeline.BindStorageBuffer(outputBuffer);
            var playerData = new PlayerData
            {
                position = position,
                direction = direction
            };
            playerBufferData.CopyToGPUBuffer(playerData, playerBufferType);
            playerBuffer.SetData(playerBufferData);
            foreach (Vector3I blockPosition in candidates)
            {
                Block block = world.GetBlock(blockPosition);
                var blockData = new BlockData
                {
                    position = blockPosition,
                    hasModel = false,
                    model = null
                };
                if (block.Model != null)
                {
                    blockData.hasModel = true;
                    var modelVertices = block.Model.Vertices;
                    var modelIndices = block.Model.Indices;
                    var vertices = new List<Vertex>();
                    foreach (var vertex in modelVertices)
                    {
                        vertices.Add(new Vertex
                        {
                            position = vertex.Position,
                            normal = vertex.Normal,
                        });
                    }
                    blockData.model = new ModelData
                    {
                        vertexCount = modelVertices.Count,
                        indexCount = modelIndices.Count,
                        vertices = vertices.ToArray(),
                        indices = new List<uint>(modelIndices).ToArray()
                    };
                }
                blockBufferData.CopyToGPUBuffer(blockData, blockBufferType);
                blockBuffer.SetData(blockBufferData);
                int indexCount = blockData.model?.indexCount ?? 36;
                computePipeline.Dispatch((uint)indexCount / 3);
                // todo: take output
                outputBufferData.Zero();
                outputBuffer.SetData(outputBufferData);
            }
        }
        public Block Block => mBlock ?? throw new NullReferenceException();
        private readonly Type mBlockType;
        private Block? mBlock;
        protected Factory? mFactory;
        private Texture? mImage;
        private static Shader? mGetFaceDirection = null;
    }
}
