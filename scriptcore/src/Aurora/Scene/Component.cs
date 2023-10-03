namespace Aurora
{
    public abstract class Component {
        public Entity Entity {get; internal set;}
    }

    public class MeshRenderer : Component {
        Mesh Mesh {get;}

    }

    public class Billboard : Component
    {
        public Vector4 Color;
        public Texture Texture;

    }

    public class ScriptComponent : Component
    {
        public string Name;
    }

    public class PointLight : Component
    {
        public Vector3 Color;
        public bool Enabled;
        public float Power;

        public int LightId;
    }

    public class Skybox : Component
    {

    }

}