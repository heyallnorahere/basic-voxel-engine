using System;
using System.Runtime.InteropServices;

namespace BasicVoxelEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public float X { get; set; }
        public float Y { get; set; }
        public float Z { get; set; }
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
        public float TaxicabLength => Math.Abs(X) + Math.Abs(Y) + Math.Abs(Z);
        public Vector3 Dot(Vector3 vector) => new Vector3(X * vector.X, Y * vector.Y, Z * vector.Z);
        public static Vector3 operator+(Vector3 vector1, Vector3 vector2) => new Vector3(vector1.X + vector2.X, vector1.Y + vector2.Y, vector1.Z + vector2.Z);
        public static Vector3 operator+(Vector3 vector, float scalar) => vector + new Vector3(scalar);
        public static Vector3 operator+(float scalar, Vector3 vector) => new Vector3(scalar) + vector;
        public static Vector3 operator-(Vector3 vector) => new Vector3(-vector.X, -vector.Y, -vector.Z);
        public static Vector3 operator-(Vector3 vector1, Vector3 vector2) => vector1 + -vector1;
        public static Vector3 operator-(Vector3 vector, float scalar) => vector - new Vector3(scalar);
        public static Vector3 operator-(float scalar, Vector3 vector) => new Vector3(scalar) - vector;
        public static Vector3 operator*(Vector3 vector1, Vector3 vector2) => vector1.Dot(vector2);
        public static Vector3 operator*(Vector3 vector, float scalar) => vector * new Vector3(scalar);
        public static Vector3 operator*(float scalar, Vector3 vector) => new Vector3(scalar) * vector;
        public static Vector3 operator/(Vector3 vector1, Vector3 vector2) => new Vector3(vector1.X / vector2.X, vector1.Y / vector2.Y, vector1.Z / vector2.Z);
        public static Vector3 operator/(Vector3 vector, float scalar) => vector / new Vector3(scalar);
        public static Vector3 operator/(float scalar, Vector3 vector) => new Vector3(scalar) / vector;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3I
    {
        public int X { get; set; }
        public int Y { get; set; }
        public int Z { get; set; }
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
        public Vector3I Dot(Vector3I vector) => new Vector3I(X * vector.X, Y * vector.Y, Z * vector.Z);
        public static Vector3I operator+(Vector3I vector1, Vector3I vector2) => new Vector3I(vector1.X + vector2.X, vector1.Y + vector2.Y, vector1.Z + vector2.Z);
        public static Vector3I operator+(Vector3I vector, int scalar) => vector + new Vector3I(scalar);
        public static Vector3I operator+(int scalar, Vector3I vector) => new Vector3I(scalar) + vector;
        public static Vector3I operator-(Vector3I vector) => new Vector3I(-vector.X, -vector.Y, -vector.Z);
        public static Vector3I operator-(Vector3I vector1, Vector3I vector2) => vector1 + -vector1;
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