using BasicVoxelEngine.Graphics;

namespace BasicVoxelEngine.Content.Blocks
{
    [AutoRegister("bve:model_block", PreferredIndex = 3)]
    public class ModelBlock : Block
    {
        protected override void Load(Factory factory, NamespacedName namespacedName)
        {
            string modelPath = AssetManager.GetAssetPath($"model:{namespacedName}.obj");
            mModel = Model.LoadModel(modelPath, factory);
        }
        public override string FriendlyName => "Model block";
        public override Model? Model => mModel;
        private Model? mModel;
    }
}
