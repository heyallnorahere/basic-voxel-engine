using BasicVoxelEngine.Graphics;
using BasicVoxelEngine.Lighting;

namespace BasicVoxelEngine
{
    public abstract class Block : RegisteredObject<Block>
    {
        public Block() { }
        /// <summary>
        /// The opacity of this block. Default is 100% (1.0).
        /// </summary>
        public virtual float Opacity => 1f;
        /// <summary>
        /// If this block is solid or not. Default is true.
        /// </summary>
        public virtual bool Solid => true;
        /// <summary>
        /// The model that represents this block. Default is null.
        /// Use <see cref="Model.LoadModel(string, Factory)" /> to load a model.
        /// </summary>
        public virtual Model? Model => null;
        /// <summary>
        /// The light that this block emits.
        /// </summary>
        public virtual Light? Light => null;
    }
}