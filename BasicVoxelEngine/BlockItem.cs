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
        private Vector3I GetFaceDirection(Vector3 direction)
        {
            if (mGetFaceDirection == null)
            {
                return new Vector3I(0);
            }
            var computePipeline = mFactory.CreateComputePipeline(mGetFaceDirection);
            // todo: send data to created compute pipeline
            computePipeline.Dispatch();
            return new Vector3I(0);
        }
        private void PlaceBlock(ClickActionArgs args)
        {
            Entity user = args.User;
            var cameraComponent = user.GetComponent<CameraComponent>();
            var transformComponent = user.GetComponent<TransformComponent>();
            var position = transformComponent.Translation;
            var direction = cameraComponent.Direction;
            var world = Application.World;
            for (var offset = new Vector3(0f); offset.Length < args.Reach; offset += direction)
            {
                // todo: improve "collision detection"
                var newPosition = position + offset;
                var blockPosition = new Vector3I
                {
                    X = (int)Math.Ceiling(newPosition.X),
                    Y = (int)Math.Ceiling(newPosition.Y),
                    Z = (int)Math.Ceiling(newPosition.Z)
                };
                int blockType;
                world.GetBlock(blockPosition, out blockType);
                if (blockType != 0)
                {
                    world.SetBlock(blockPosition + GetFaceDirection(-direction), Block);
                }
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
