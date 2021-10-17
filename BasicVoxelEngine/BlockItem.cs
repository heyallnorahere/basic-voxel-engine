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
        public Block Block => mBlock ?? throw new NullReferenceException();
        private readonly Type mBlockType;
        private Block? mBlock;
        protected Factory? mFactory;
        private Texture? mImage;
    }
}
