using BasicVoxelEngine.Graphics;
using BasicVoxelEngine.Lighting;
using System;
using System.Runtime.CompilerServices;

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
        public virtual Light? Light => null;
        /// <summary>
        /// The friendly name of this block.
        /// </summary>
        public abstract string FriendlyName { get; }
    }
}