using BasicVoxelEngine.Graphics;
using System;

namespace BasicVoxelEngine
{
    public class Item : RegisteredObject<Item>
    {
        public struct ClickActionArgs
        {
            public Entity User { get; set; }
            public ItemStack? ItemStack { get; set; }
            public float Reach { get; set; }
        }
        public static class Actions
        {
            public static void Punch(ClickActionArgs args)
            {
                // todo: animation
                // todo: damage blocks or entities
            }
        }
        /// <summary>
        /// The model that represents this item.
        /// </summary>
        public virtual Model? Model => null;
        /// <summary>
        /// The texture (image) that represents this item
        /// </summary>
        public virtual Texture? Image => null;
        /// <summary>
        /// The maximum quantity of items in an <see cref="ItemStack"/>
        /// </summary>
        public virtual int MaxStack => 64;
        /// <summary>
        /// The function that is called when the player left-clicks
        /// </summary>
        public virtual Action<ClickActionArgs> LeftClick => Actions.Punch;
        /// <summary>
        /// The function that is called when the player right-clicks
        /// </summary>
        public virtual Action<ClickActionArgs>? RightClick => null;
    }
}
