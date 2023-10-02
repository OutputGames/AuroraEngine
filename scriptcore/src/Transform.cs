 using System;

 namespace Aurora
{
    public class Transform
    {

        public Entity Entity;

        public Vector3 Position
        {
            get
            {
                if (Entity != null)
                {
                    //Console.WriteLine("Getting position");
                    InternalCalls.TransformGetPosition(Entity.Id, out Vector3 translation);
                    //Console.WriteLine("Got position successfully!");
                    return translation;
                }
                else
                {
                    Console.WriteLine("Entity is null");
                    return new Vector3(0.0f);
                }
            }
            set
            {
                if (Entity != null)
                {
                    //Console.WriteLine("Setting position");
                    InternalCalls.TransformSetPosition(Entity.Id, ref value);
                    //Console.WriteLine("Set Position successfully");
                }
                else
                {
                    //Console.WriteLine("Entity is null");
                    Position = value;
                }
            }
        }

        public Transform(Entity entity)
        {
            Entity = entity;
            Position = new Vector3(0.0f);
        }
    }
}