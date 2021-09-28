using System;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2
    {
        public float X { get; set; }
        public float Y { get; set; }
        public Vector2(float scalar)
        {
            X = Y = scalar;
        }
        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }
        public float Length => Convert.ToSingle(Math.Sqrt(X * X + Y * Y));
        /// <summary>
        /// The <see cref="Length" />, squared
        /// </summary>
        public float Length2
        {
            get
            {
                // to avoid calculating twice
                float length = Length;
                return length * length;
            }
        }
        public Vector2 Normalized => this / Length;
        public override int GetHashCode()
        {
            return (X.GetHashCode() << 1) ^ Y.GetHashCode();
        }
        public float TaxicabLength => Math.Abs(X) + Math.Abs(Y);
        public Vector2 Degrees
        {
            get
            {
                return new Vector2
                {
                    X = X.ToDegrees(),
                    Y = Y.ToDegrees()
                };
            }
        }
        public Vector2 Radians
        {
            get
            {
                return new Vector2
                {
                    X = X.ToRadians(),
                    Y = Y.ToRadians()
                };
            }
        }
        public Vector2 Dot(Vector2 vector) => new Vector2(X * vector.X, Y * vector.Y);
        public static Vector2 operator+(Vector2 vector1, Vector2 vector2) => new Vector2(vector1.X + vector2.X, vector1.Y + vector2.Y);
        public static Vector2 operator+(Vector2 vector, float scalar) => vector + new Vector2(scalar);
        public static Vector2 operator+(float scalar, Vector2 vector) => new Vector2(scalar) + vector;
        public static Vector2 operator-(Vector2 vector) => new Vector2(-vector.X, -vector.Y);
        public static Vector2 operator-(Vector2 vector1, Vector2 vector2) => vector1 + -vector2;
        public static Vector2 operator-(Vector2 vector, float scalar) => vector - new Vector2(scalar);
        public static Vector2 operator-(float scalar, Vector2 vector) => new Vector2(scalar) - vector;
        public static Vector2 operator*(Vector2 vector1, Vector2 vector2) => vector1.Dot(vector2);
        public static Vector2 operator*(Vector2 vector, float scalar) => vector * new Vector2(scalar);
        public static Vector2 operator*(float scalar, Vector2 vector) => new Vector2(scalar) * vector;
        public static Vector2 operator/(Vector2 vector1, Vector2 vector2) => new Vector2(vector1.X / vector2.X, vector1.Y / vector2.Y);
        public static Vector2 operator/(Vector2 vector, float scalar) => vector / new Vector2(scalar);
        public static Vector2 operator/(float scalar, Vector2 vector) => new Vector2(scalar) / vector;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2I
    {
        public int X { get; set; }
        public int Y { get; set; }
        public Vector2I(int scalar)
        {
            X = Y = scalar;
        }
        public Vector2I(int x, int y)
        {
            X = x;
            Y = y;
        }
        public float Length => Convert.ToSingle(Math.Sqrt(X * X + Y * Y));
        /// <summary>
        /// The <see cref="Length" />, squared
        /// </summary>
        public float Length2
        {
            get
            {
                // to avoid calculating twice
                float length = Length;
                return length * length;
            }
        }
        public override int GetHashCode()
        {
            return (X.GetHashCode() << 1) ^ Y.GetHashCode();
        }
        public int TaxicabLength => Math.Abs(X) + Math.Abs(Y);
        public Vector2I Dot(Vector2I vector) => new Vector2I(X * vector.X, Y * vector.Y);
        public static Vector2I operator+(Vector2I vector1, Vector2I vector2) => new Vector2I(vector1.X + vector2.X, vector1.Y + vector2.Y);
        public static Vector2I operator+(Vector2I vector, int scalar) => vector + new Vector2I(scalar);
        public static Vector2I operator+(int scalar, Vector2I vector) => new Vector2I(scalar) + vector;
        public static Vector2I operator-(Vector2I vector) => new Vector2I(-vector.X, -vector.Y);
        public static Vector2I operator-(Vector2I vector1, Vector2I vector2) => vector1 + -vector2;
        public static Vector2I operator-(Vector2I vector, int scalar) => vector - new Vector2I(scalar);
        public static Vector2I operator-(int scalar, Vector2I vector) => new Vector2I(scalar) - vector;
        public static Vector2I operator*(Vector2I vector1, Vector2I vector2) => vector1.Dot(vector2);
        public static Vector2I operator*(Vector2I vector, int scalar) => vector * new Vector2I(scalar);
        public static Vector2I operator*(int scalar, Vector2I vector) => new Vector2I(scalar) * vector;
        public static Vector2I operator/(Vector2I vector1, Vector2I vector2) => new Vector2I(vector1.X / vector2.X, vector1.Y / vector2.Y);
        public static Vector2I operator/(Vector2I vector, int scalar) => vector / new Vector2I(scalar);
        public static Vector2I operator/(int scalar, Vector2I vector) => new Vector2I(scalar) / vector;
    }
}