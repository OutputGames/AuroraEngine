namespace Aurora
{
    public class Vector4
    {
        public float x,y,z,w;

		public Vector4(float scalar)
		{
			x = scalar;
			y = scalar;
			z = scalar;
			w = scalar;
		}

		public Vector4(float x, float y, float z, float w)
		{
			x = x;
			y = y;
			z = z;
			w = w;
		}

		public Vector4(Vector2 xy,  Vector2 zw)
		{
			x = xy.x;
			y = xy.y;
            z = zw.x;
            w = zw.y;
        }

    }
}