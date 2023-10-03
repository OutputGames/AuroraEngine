namespace Aurora
{
    public class Vector4
    {
        public float X,Y,Z,W;

		public Vector4(float scalar)
		{
			X = scalar;
			Y = scalar;
			Z = scalar;
			W = scalar;
		}

		public Vector4(float x, float y, float z, float w)
		{
			X = x;
			Y = y;
			Z = z;
			W = w;
		}

		public Vector4(Vector2 xy,  Vector2 zw)
		{
			X = xy.X;
			Y = xy.Y;
            Z = zw.X;
            W = zw.Y;
        }

    }
}