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

    }
}