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

        public Material Material;

         
        internal Entity(uint id)
        {
            Id = id;
            Transform = new Transform(this);
            Material = new Material(this);
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

        public bool HasComponent<T>() where T : Component, new() {
            Type compType = typeof(T);

            return InternalCalls.EntityHasComponent(Id, compType);
        }


        public T GetComponent<T>() where T : Component, new() {
            if (!HasComponent<T>())
                return null;

            T component = new T() {Entity = this };
            return component;
        }

        public static Entity Create()
        {
            return InternalCalls.CreateEntity("new entity");
        }
    }
}