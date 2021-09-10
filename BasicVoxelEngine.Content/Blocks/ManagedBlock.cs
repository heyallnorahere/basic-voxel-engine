using BasicVoxelEngine.Graphics;
using BasicVoxelEngine.Lighting;

namespace BasicVoxelEngine.Content.Blocks
{
    [AutoRegister("bve:managed_block")]
    public class ManagedBlock : Block
    {
        public override string FriendlyName => "Managed block";
        protected override void Load(Factory factory, NamespacedName namespacedName)
        {
            mModel = Model.LoadModel(AssetManager.GetAssetPath("model/bve:model_block.obj"), factory);
            mLight = /*new PointLight
            {
                AmbientStrength = 0.01f,
                SpecularStrength = 0.5f,
                Color = new Vector3(1f),
                Constant = 1f,
                Linear = 0.09f,
                Quadratic = 0.032f,
            }*/ null;
        }
        public override Model? Model => mModel;
        public override Light? Light => mLight;
        private Model? mModel;
        private Light? mLight;
    }
}