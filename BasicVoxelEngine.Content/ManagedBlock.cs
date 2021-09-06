using BasicVoxelEngine.Graphics;
using BasicVoxelEngine.Lighting;
using System;

namespace BasicVoxelEngine.Content
{
    [AutoRegister("bve:managed_block")]
    public class ManagedBlock : Block
    {
        public override string FriendlyName => "Managed block";
        protected override void Load(Factory factory, NamespacedName namespacedName)
        {
            mModel = Model.LoadModel(AssetManager.GetAssetPath("model/bve:model_block.obj"), factory);
            mLight = new Spotlight
            {
                AmbientStrength = 0.01f,
                SpecularStrength = 0.5f,
                Color = new Vector3(1f),
                Cutoff = Convert.ToSingle(Math.Cos(30 * (Math.PI / 180))),
                Direction = new Vector3(1f, 0f, 0f)
            };
        }
        public override Model? Model => mModel;
        public override Light? Light => mLight;
        private Model? mModel;
        private Light? mLight;
    }
}