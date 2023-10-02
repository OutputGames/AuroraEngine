using System;

namespace Aurora {
    class MeshRenderer : Entity
    {

        public void OnInit()
        {
            Console.WriteLine("Initialized MeshRenderer");
            Transform.Position = new Vector3(0.0f);
        }

        public void OnUpdate(float dt)
        {
            //Console.WriteLine(dt);
            //Console.WriteLine("entity name: "+Name+", id: "+Id);

            Vector3 pos = Transform.Position;

            pos.Y += 0.01f;

            Transform.Position = pos;
        }
    }
}