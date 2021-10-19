using BasicVoxelEngine.Components;
using BasicVoxelEngine.Graphics;
using System;

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
        private static Vector3I GetFaceDirection(Vector3 direction)
        {
            // good enough for now
            double pitch = Math.Asin(direction.Y);
            double cosineOfPitch = Math.Cos(pitch);
            double yaw = Math.Atan2(direction.Z / cosineOfPitch, direction.X / cosineOfPitch);
            if (Math.Abs(pitch).ToDegrees() > 45D)
            {
                return new Vector3I
                {
                    X = 0,
                    Y = direction.Y > 0f ? 1 : -1,
                    Z = 0
                };
            }
            else if (Math.Abs(yaw).ToDegrees() % 90D > 45D)
            {
                return new Vector3I
                {
                    X = direction.X > 0f ? 1 : -1,
                    Y = 0,
                    Z = 0
                };
            }
            else
            {
                return new Vector3I
                {
                    X = 0,
                    Y = 0,
                    Z = direction.Z > 0f ? 1 : -1
                };
            }
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
    }
}
