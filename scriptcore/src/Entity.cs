using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Aurora
{
    public class Entity
    {
        public readonly uint Id;
        public string Name { get; set; }
        public bool Enabled { get; set; }

        public Transform Transform { get; set; }

         
        internal Entity(uint id)
        {
            Id = id;
            Transform = new Transform(this);
            Console.WriteLine("Created entity with id: "+Id);

        }

        protected Entity()
        { Id = 0;}

        public void OnInit()
        {
        }

        public void OnUpdate(float dt)
        {

        }

        public static Entity Create()
        {
            return InternalCalls.CreateEntity("new entity");
        }
    }
}