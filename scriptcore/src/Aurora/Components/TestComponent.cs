using System;
using System.Collections.Generic;

namespace Aurora {
    class TestComponent : Entity
    {

        public void OnInit()
        {
            bool hasComp = HasComponent<MeshRenderer>();

            Console.WriteLine("Entity has MeshRenderer = "+hasComp);

            var dictionary = new Dictionary<string, object>();
            InternalCalls.MaterialGetUniforms(Id, out dictionary);

            Console.WriteLine(Material.Uniforms["albedo"]);

            Material.Uniforms["albedo"] = new Vector3(1, 0, 0);
        }

        public void OnUpdate(float dt)
        {
            //Console.WriteLine(dt);
            //Console.WriteLine("entity name: "+Name+", id: "+Id);

            Transform.Position += new Vector3(0.01f,0,0);
        }
    }
}