using BasicVoxelEngine.Graphics;

namespace BasicVoxelEngine.Content.Items
{
    [AutoRegister("bve:test_item")]
    public sealed class TestItem : Item
    {
        protected override void Load(Factory factory, NamespacedName namespacedName)
        {
            mImage = factory.CreateTexture(AssetManager.GetAssetPath($"item:{namespacedName}.png"));
        }
        public override string FriendlyName => "Test item";
        public override Texture Image => mImage;
        private Texture? mImage;
    }
}
