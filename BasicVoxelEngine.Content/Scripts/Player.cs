using BasicVoxelEngine.Components;
using BasicVoxelEngine.Content.Items;
using BasicVoxelEngine.Graphics;
using System;

namespace BasicVoxelEngine.Content.Scripts
{
    public sealed class Player : Script
    {
        const int InventoryWidth = 9;
        const int InventoryHeight = 3;
        public Player()
        {
            int inventorySize = InventoryWidth * (InventoryHeight + 1);
            mInventory = new ItemStack[inventorySize];
            for (int i = 0; i < inventorySize; i++)
            {
                mInventory[i] = null;
            }
            mHotbarIndex = 0;
            mReach = 5f; // for now
        }
        public override void OnAttach()
        {
            mCurrentMouseOffset = new Vector2(0f);
            mCameraSensitivity = 0.1f;
            GetComponent<TransformComponent>().Translation = new Vector3(5f, 64f, 5f);
            AddComponent<CameraComponent>().Direction = new Vector3(-1f);
            Entity uiControllerEntity = Parent.World.CreateEntity();
            uiControllerEntity.AddComponent<ScriptComponent>().Bind<UserInterfaceController>();
            uiControllerEntity.RemoveComponent<TransformComponent>();
            UIController.Instance = uiControllerEntity;
            var itemRegister = Registry.GetRegister<Item>();
            var hotbarItems = new Item[]
            {
                itemRegister.GetInstance<Grass>(),
                itemRegister.GetInstance<Dirt>(),
                itemRegister.GetInstance<TestLight>(),
                itemRegister.GetInstance<TestItem>(),
            };
            for (int i = 0; i < (hotbarItems.Length > InventoryWidth ? InventoryWidth : hotbarItems.Length); i++)
            {
                mInventory[GetInventoryIndex(i, InventoryHeight)] = new ItemStack(hotbarItems[i]);
            }
        }
        public override void Update()
        {
            TakeInput();
            UpdateCameraDirection();
            RenderUI();
        }
        private void RenderUI()
        {
            var scriptComponent = UIController.Instance.GetComponent<ScriptComponent>();
            if (!scriptComponent.HasScriptBound<UIController>())
            {
                return;
            }
            var uiController = scriptComponent.GetScript<UIController>();
            ItemStack? heldItem = mInventory[GetInventoryIndex(mHotbarIndex, InventoryHeight)];
            if (heldItem != null)
            {
                Texture? image = heldItem.Item.Image;
                if (image != null)
                {
                    var size = new Vector2I(64);
                    Vector2I windowSize = Application.Window.FramebufferSize;
                    var origin = new Vector2I
                    {
                        X = (windowSize.X - size.X) / 2,
                        Y = windowSize.Y - size.Y
                    };
                    uiController.DrawQuad(image, origin, size);
                }
            }
        }
        private void TakeInput()
        {
            var inputManager = Application.InputManager;
            var camera = GetComponent<CameraComponent>();
            var transform = GetComponent<TransformComponent>();
            float playerSpeed = 2.5f * (float)Application.GetDeltaTime();
            var horizontal = new Vector3(1f, 0f, 1f);
            var speed = horizontal * playerSpeed;
            Vector3 forward = camera.Direction * speed;
            Vector3 up = camera.Up * playerSpeed;
            Vector3 right = camera.Direction.Cross(camera.Up).Normalized * speed;
            if (inputManager.GetKey(Key.W).Held)
            {
                transform.Translation += forward;
            }
            if (inputManager.GetKey(Key.S).Held)
            {
                transform.Translation -= forward;
            }
            if (inputManager.GetKey(Key.A).Held)
            {
                transform.Translation -= right;
            }
            if (inputManager.GetKey(Key.D).Held)
            {
                transform.Translation += right;
            }
            if (inputManager.GetKey(Key.SPACE).Held)
            {
                transform.Translation += up;
            }
            if (inputManager.GetKey(Key.LEFT_SHIFT).Held)
            {
                transform.Translation -= up;
            }
            if (inputManager.GetKey(Key.Q).Down)
            {
                Application.Quit();
            }
            if (inputManager.GetKey(Key.E).Down)
            {
                inputManager.MouseEnabled = !inputManager.MouseEnabled;
            }
            if (inputManager.GetMouseButton(InputManager.LeftMouseButton).Down)
            {
                LeftClick();
            }
            if (inputManager.GetMouseButton(InputManager.RightMouseButton).Down)
            {
                RightClick();
            }
            mCurrentMouseOffset = inputManager.Mouse;
            for (int i = 0; i < 9; i++)
            {
                Key key = Key.N1 + i;
                if (inputManager.GetKey(key).Down)
                {
                    mHotbarIndex = i;
                }
            }
        }
        private void LeftClick()
        {
            var stack = mInventory[GetInventoryIndex(mHotbarIndex, InventoryHeight)];
            if (stack != null)
            {
                stack.Item.LeftClick(new Item.ClickActionArgs
                {
                    User = Parent,
                    ItemStack = stack,
                    Reach = mReach
                });
            }
            else
            {
                Item.Actions.Punch(new Item.ClickActionArgs
                {
                    User = Parent,
                    ItemStack = null,
                    Reach = mReach
                });
            }
        }
        private void RightClick()
        {
            int inventoryIndex = GetInventoryIndex(mHotbarIndex, InventoryHeight);
            var stack = mInventory[inventoryIndex];
            if (stack?.Item?.RightClick != null)
            {
                stack.Item.RightClick(new Item.ClickActionArgs
                {
                    User = Parent,
                    ItemStack = stack,
                    Reach = mReach
                });
                if (stack.Quantity <= 0)
                {
                    mInventory[inventoryIndex] = null;
                }
                return;
            }
            // todo: other stuff
        }
        private void UpdateCameraDirection()
        {
            Vector2 offset = mCurrentMouseOffset * mCameraSensitivity;
            var camera = GetComponent<CameraComponent>();
            var direction = camera.Direction;
            double x = Math.Asin(direction.Y);
            double factor = Math.Cos(x);
            double y = Math.Atan2(direction.Z / factor, direction.X / factor);
            var radians = new Vector2
            {
                X = (float)x,
                Y = (float)y
            };
            var angle = radians.Degrees;
            var cameraOffset = new Vector2(offset.Y, offset.X);
            if (camera.Up.Y < 0f)
            {
                cameraOffset *= -1f;
            }
            angle += cameraOffset;
            if (angle.X > 89f)
            {
                angle.X = 89f;
            }
            if (angle.X < -89f)
            {
                angle.X = -89f;
            }
            radians = angle.Radians;
            var newDirection = new Vector3
            {
                X = (float)(Math.Cos(radians.X) * Math.Cos(radians.Y)),
                Y = (float)Math.Sin(radians.X),
                Z = (float)(Math.Cos(radians.X) * Math.Sin(radians.Y))
            };
            camera.Direction = newDirection.Normalized;
        }
        private int GetInventoryIndex(int x, int y) => (y * InventoryWidth) + x;
        private float mCameraSensitivity;
        private Vector2 mCurrentMouseOffset;
        private int mHotbarIndex;
        private readonly ItemStack?[] mInventory;
        private float mReach;
    }
}