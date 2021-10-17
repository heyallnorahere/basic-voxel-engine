using System;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public float X;
        public float Y;
        public float Z;
        public Vector3(float scalar)
        {
            X = Y = Z = scalar;
        }
        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }
        public Vector3(Vector2 vector, float z)
        {
            X = vector.X;
            Y = vector.Y;
            Z = z;
        }
        public Vector3(float x, Vector2 vector)
        {
            X = x;
            Y = vector.X;
            Z = vector.Y;
        }
        public float Length => Convert.ToSingle(Math.Sqrt(X * X + Y * Y + Z * Z));
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
        public Vector3 Normalized => this / Length;
        public override int GetHashCode()
        {
            return (X.GetHashCode() << 2) ^ (Y.GetHashCode() << 1) ^ Z.GetHashCode();
        }
        public float TaxicabLength => Math.Abs(X) + Math.Abs(Y) + Math.Abs(Z);
        public Vector3 Degrees
        {
            get
            {
                return new Vector3
                {
                    X = X.ToDegrees(),
                    Y = Y.ToDegrees(),
                    Z = Z.ToDegrees()
                };
            }
        }
        public Vector3 Radians
        {
            get
            {
                return new Vector3
                {
                    X = X.ToRadians(),
                    Y = Y.ToRadians(),
                    Z = Z.ToRadians()
                };
            }
        }
        public Vector2 XY => new Vector2(X, Y);
        public Vector2 YZ => new Vector2(Y, Z);
        public Vector2 XZ => new Vector2(X, Z);
        public Vector3 Dot(Vector3 vector) => new Vector3(X * vector.X, Y * vector.Y, Z * vector.Z);
        public Vector3 Cross(Vector3 vector)
        {
            return new Vector3
            {
                X = Y * vector.Z - Z * vector.Y,
                Y = Z * vector.X - X * vector.Z,
                Z = X * vector.Y - Y * vector.X
            };
        }
        public static Vector3 operator+(Vector3 vector1, Vector3 vector2) => new Vector3(vector1.X + vector2.X, vector1.Y + vector2.Y, vector1.Z + vector2.Z);
        public static Vector3 operator+(Vector3 vector, float scalar) => vector + new Vector3(scalar);
        public static Vector3 operator+(float scalar, Vector3 vector) => new Vector3(scalar) + vector;
        public static Vector3 operator-(Vector3 vector) => new Vector3(-vector.X, -vector.Y, -vector.Z);
        public static Vector3 operator-(Vector3 vector1, Vector3 vector2) => vector1 + -vector2;
        public static Vector3 operator-(Vector3 vector, float scalar) => vector - new Vector3(scalar);
        public static Vector3 operator-(float scalar, Vector3 vector) => new Vector3(scalar) - vector;
        public static Vector3 operator*(Vector3 vector1, Vector3 vector2) => vector1.Dot(vector2);
        public static Vector3 operator*(Vector3 vector, float scalar) => vector * new Vector3(scalar);
        public static Vector3 operator*(float scalar, Vector3 vector) => new Vector3(scalar) * vector;
        public static Vector3 operator/(Vector3 vector1, Vector3 vector2) => new Vector3(vector1.X / vector2.X, vector1.Y / vector2.Y, vector1.Z / vector2.Z);
        public static Vector3 operator/(Vector3 vector, float scalar) => vector / new Vector3(scalar);
        public static Vector3 operator/(float scalar, Vector3 vector) => new Vector3(scalar) / vector;
        public static implicit operator Vector2(Vector3 vector) => vector.XY;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3I
    {
        public int X;
        public int Y;
        public int Z;
        public Vector3I(int scalar)
        {
            X = Y = Z = scalar;
        }
        public Vector3I(int x, int y, int z)
        {
            X = x;
            Y = y;
            Z = z;
        }
        public Vector3I(Vector2I vector, int z)
        {
            X = vector.X;
            Y = vector.Y;
            Z = z;
        }
        public Vector3I(int x, Vector2I vector)
        {
            X = x;
            Y = vector.X;
            Z = vector.Y;
        }
        public float Length => Convert.ToSingle(Math.Sqrt(X * X + Y * Y + Z * Z));
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
            return (X.GetHashCode() << 2) ^ (Y.GetHashCode() << 1) ^ Z.GetHashCode();
        }
        public int TaxicabLength => Math.Abs(X) + Math.Abs(Y) + Math.Abs(Z);
        public Vector2I XY => new Vector2I(X, Y);
        public Vector2I YZ => new Vector2I(Y, Z);
        public Vector2I XZ => new Vector2I(X, Z);
        public Vector3I Dot(Vector3I vector) => new Vector3I(X * vector.X, Y * vector.Y, Z * vector.Z);
        public static Vector3I operator+(Vector3I vector1, Vector3I vector2) => new Vector3I(vector1.X + vector2.X, vector1.Y + vector2.Y, vector1.Z + vector2.Z);
        public static Vector3I operator+(Vector3I vector, int scalar) => vector + new Vector3I(scalar);
        public static Vector3I operator+(int scalar, Vector3I vector) => new Vector3I(scalar) + vector;
        public static Vector3I operator-(Vector3I vector) => new Vector3I(-vector.X, -vector.Y, -vector.Z);
        public static Vector3I operator-(Vector3I vector1, Vector3I vector2) => vector1 + -vector2;
        public static Vector3I operator-(Vector3I vector, int scalar) => vector - new Vector3I(scalar);
        public static Vector3I operator-(int scalar, Vector3I vector) => new Vector3I(scalar) - vector;
        public static Vector3I operator*(Vector3I vector1, Vector3I vector2) => vector1.Dot(vector2);
        public static Vector3I operator*(Vector3I vector, int scalar) => vector * new Vector3I(scalar);
        public static Vector3I operator*(int scalar, Vector3I vector) => new Vector3I(scalar) * vector;
        public static Vector3I operator/(Vector3I vector1, Vector3I vector2) => new Vector3I(vector1.X / vector2.X, vector1.Y / vector2.Y, vector1.Z / vector2.Z);
        public static Vector3I operator/(Vector3I vector, int scalar) => vector / new Vector3I(scalar);
        public static Vector3I operator/(int scalar, Vector3I vector) => new Vector3I(scalar) / vector;
    }
}