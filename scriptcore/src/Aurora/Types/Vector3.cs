using System;

namespace Aurora
{
    public struct Vector3
    {
        public float X,Y,Z;

		public Vector3(float scalar)
		{
			X = scalar;
			Y = scalar;
			Z = scalar;
		}

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public Vector3(Vector2 xy, float z)
		{
			X = xy.X;
			Y = xy.Y;
			this.Z = z;
		}


        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        }

        public static Vector3 operator *(Vector3 a, Vector3 b)
        {
            return new Vector3(a.X * b.X, a.Y * b.Y, a.Z * b.Z);
        }

        public static Vector3 operator *(Vector3 vector, float scalar)
        {
            return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

        public override string ToString()
        {
            return X + "," + Y + "," + Z;
        }

        public static Vector3 Parse(string s)
        {

            string[] ps = s.Split(',');

            return new Vector3(float.Parse(ps[0]), float.Parse(ps[1]), float.Parse(ps[2]));
        }

    }
}