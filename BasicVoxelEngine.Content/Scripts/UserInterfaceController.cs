using BasicVoxelEngine.Graphics;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine.Content.Scripts
{
    public abstract class UIController : Script
    {
        public abstract void DrawQuad(Texture texture, Vector2I origin, Vector2I size);
        public static Entity Instance { get; internal set; } = new Entity();
    }
    internal sealed class UserInterfaceController : UIController
    {
        public UserInterfaceController()
        {
            mDrawnQuads = new Dictionary<Texture, List<Quad>>();
            Factory factory = Application.Factory;
            mShader = factory.CreateShader(new string[] { AssetManager.GetAssetPath("shaders:ui.glsl") });
        }
        private Vector2 ConvertCoords(Vector2 coords)
        {
            Vector2 newCoords = coords;
            if (Application.Factory.API == GraphicsAPI.OPENGL)
            {
                newCoords.Y = 1f - newCoords.Y;
            }
            return (newCoords - 0.5f) * 2f;
        }
        public override void DrawQuad(Texture texture, Vector2I origin, Vector2I size)
        {
            Texture key = texture;
            foreach (Texture existingKey in mDrawnQuads.Keys)
            {
                if (Helpers.AreRefsEqual(key.Address, existingKey.Address))
                {
                    key = existingKey;
                }
            }
            if (!mDrawnQuads.ContainsKey(key))
            {
                if (mDrawnQuads.Count >= 30)
                {
                    throw new ArgumentException("Cannot render more than 30 textures at once!");
                }
                mDrawnQuads.Add(key, new List<Quad>());
            }
            Vector2 windowSize = new Vector2(Application.Window.FramebufferSize);
            var quad = new Quad()
            {
                TopLeft = ConvertCoords(new Vector2(origin) / windowSize),
                BottomRight = ConvertCoords(new Vector2(origin + size) / windowSize)
            };
            mDrawnQuads[key].Add(quad);
        }
        public override void NewFrame()
        {
            mDrawnQuads.Clear();
        }
        public override void Render(Renderer renderer)
        {
            if (mDrawnQuads.Count <= 0)
            {
                return;
            }
            mCommandList = renderer.CreateCommandList();
            var textures = new List<Texture>();
            foreach (var texture in mDrawnQuads.Keys)
            {
                foreach (Quad quad in mDrawnQuads[texture])
                {
                    Mesh mesh = CreateQuadMesh(quad, textures.Count);
                    mCommandList.AddMesh(mesh);
                }
                textures.Add(texture);
            }
            int stride = Marshal.SizeOf(typeof(Vertex));
            mCommandList.Close(new List<VertexAttribute>
            {
                new VertexAttribute
                {
                    Stride = stride,
                    Offset = (int)Marshal.OffsetOf(typeof(Vertex), nameof(Vertex.Position)),
                    Type = VertexAttributeType.Vector2,
                    Normalize = false
                },
                new VertexAttribute
                {
                    Stride = stride,
                    Offset = (int)Marshal.OffsetOf(typeof(Vertex), nameof(Vertex.UV)),
                    Type = VertexAttributeType.Vector2,
                    Normalize = false
                },
                new VertexAttribute
                {
                    Stride = stride,
                    Offset = (int)Marshal.OffsetOf(typeof(Vertex), nameof(Vertex.TextureIndex)),
                    Type = VertexAttributeType.Int,
                    Normalize = false
                }
            });
            renderer.SetShader(mShader);
            for (int i = 0; i < textures.Count; i++)
            {
                renderer.SetTexture(i, textures[i]);
            }
            renderer.Render(mCommandList, Application.Window.Context);
        }
        private static Mesh CreateQuadMesh(Quad quad, int textureIndex)
        {
            var mesh = new Mesh();
            mesh.SetVertices(new List<Vertex>
            {
                new Vertex
                {
                    Position = new Vector2(quad.TopLeft.X, quad.BottomRight.Y),
                    UV = new Vector2(0f, 0f),
                    TextureIndex = textureIndex
                },
                new Vertex
                {
                    Position = quad.BottomRight,
                    UV = new Vector2(1f, 0f),
                    TextureIndex = textureIndex
                },
                new Vertex
                {
                    Position = new Vector2(quad.BottomRight.X, quad.TopLeft.Y),
                    UV = new Vector2(1f, 1f),
                    TextureIndex = textureIndex
                },
                new Vertex
                {
                    Position = quad.TopLeft,
                    UV = new Vector2(0f, 1f),
                    TextureIndex = textureIndex
                }
            });
            mesh.SetIndices(new List<uint>()
            {
                0, 1, 3,
                1, 2, 3
            });
            return mesh;
        }
        [StructLayout(LayoutKind.Sequential)]
        private struct Vertex
        {
            public Vector2 Position;
            public Vector2 UV;
            public int TextureIndex;
        }
        private struct Quad
        {
            public Vector2 TopLeft { get; set; }
            public Vector2 BottomRight { get; set; }
        }
        private readonly Dictionary<Texture, List<Quad>> mDrawnQuads;
        private readonly Shader mShader;
        private CommandList? mCommandList = null;
    }
}
