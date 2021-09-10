using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
        public override string FriendlyName => "Model Block";
        public override Model? Model => mModel;
        private Model? mModel;
    }
}
