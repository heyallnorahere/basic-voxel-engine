namespace BasicVoxelEngine
{
    public sealed class ItemStack
    {
        public ItemStack(Item item, int quantity = 1)
        {
            Item = item;
            Quantity = quantity;
        }
        public int Quantity { get; set; }
        public Item Item { get; }
    }
}
