using System;
using System.Collections.Generic;
using System.Runtime;

namespace Aurora {
    class TestComponent : Entity
    {

        public float speed;

        public void OnInit()
        {
            bool hasComp = HasComponent<MeshRenderer>();

            //Console.WriteLine(Material.GetValue("albedo", ValueType.Vec3));
        }

        public void OnUpdate(float dt)
        {
            //Console.WriteLine(dt);
            //Console.WriteLine("entity name: "+Name+", id: "+Id);

            Transform.Position += new Vector3(speed,0,0);
        }
    }
}