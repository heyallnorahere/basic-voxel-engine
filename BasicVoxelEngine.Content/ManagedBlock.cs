using BasicVoxelEngine.Graphics;
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
        }
        private Model? mModel;
    }
}