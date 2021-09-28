namespace BasicVoxelEngine
{
    public abstract class Script
    {
        public virtual void OnAttach() { }
        public abstract void Update();
        public Entity Parent { get; internal set; } = new Entity();
        protected T AddComponent<T>()
        {
            return Parent.AddComponent<T>();
        }
        protected T GetComponent<T>()
        {
            return Parent.GetComponent<T>();
        }
        protected bool HasComponent<T>()
        {
            return Parent.HasComponent<T>();
        }
        protected void RemoveComponent<T>()
        {
            Parent.RemoveComponent<T>();
        }
    }
}