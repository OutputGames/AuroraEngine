namespace Aurora
{
    public struct Vector2
    {
        public float X,Y;

		public Vector2(float scalar)
		{
			X = scalar;
			Y = scalar;
		}

		public Vector2(float x, float y)
		{
			this.X = x;
			this.Y = y;
		}

        public override string ToString()
        {
            return X + "," + Y;
        }

        public static Vector2 Parse(string s)
        {
            string[] ps = s.Split(',');

            return new Vector2(float.Parse(ps[0]), float.Parse(ps[1]));
        }

    }
}