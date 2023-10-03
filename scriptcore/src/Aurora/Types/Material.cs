using System.Collections.Generic;

namespace Aurora
{
    public class Material
    {
        public List<Texture> Textures
        {
            get
            {
                if (Entity == null)
                    return Textures;
                InternalCalls.MaterialGetTextures(Entity.Id, out List<Texture> texs);
                return texs;
            }
        }

        public Entity Entity { get; internal set; }

        public Dictionary<string, object> Uniforms
        {
            get
            {
                if (Entity == null)
                    return Uniforms;
                InternalCalls.MaterialGetUniforms(Entity.Id, out Dictionary<string, object> unifs);
                return unifs;
            }
            set
            {
                if (Entity == null)
                    return;
                InternalCalls.MaterialSetUniforms(Entity.Id, ref value);
            }
        }

        public Material(Entity entity)
        {
            Entity = entity;
        }

    }
}