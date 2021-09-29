using BasicVoxelEngine.ContentLoading;

[assembly: AssemblyContentLoader(typeof(BasicVoxelEngine.Content.BVEContentLoader))]

namespace BasicVoxelEngine.Content
{
    public sealed class BVEContentLoader : ContentLoader
    {
        protected override void OnLoaded()
        {
            Logger.Print(Logger.Severity.Info, "loaded base BVE content");
        }
    }
}