using BasicVoxelEngine.Graphics;

namespace BasicVoxelEngine
{
    public class Item : RegisteredObject<Item>
    {
        /// <summary>
        /// The model that represents this item.
        /// </summary>
        public virtual Model? Model => null;
        /// <summary>
        /// The texture (image) that represents this item
        /// </summary>
        public virtual Texture? Image => null;
        /// <summary>
        /// 
        /// </summary>
        public virtual int MaxStack => 64;
    }
}
