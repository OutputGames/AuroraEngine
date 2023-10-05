using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Aurora
{
    public enum ValueType
    {
        Bool = 0,
        Float = 1,
        Int = 2,
        Vec2 = 3,
        Vec3 = 4,
        Vec4 = 5,
    }

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
        
        public object GetValue(string name, ValueType type)
        {

            //Console.WriteLine("trying to get value: " + name + " with type: " + type.ToString());

            string val = InternalCalls.MaterialGetValue(Entity.Id, type.ToString(), name);

            if (val == "null")
            {
                Console.WriteLine("returned null.");
                return 0;
            }

            //Console.WriteLine(val);

            switch (type)
            {
                case ValueType.Bool:
                    return bool.Parse(val);
                case ValueType.Float:
                    return float.Parse(val);
                case ValueType.Int:
                    return int.Parse(val);
                case ValueType.Vec2:
                    return Vector2.Parse(val);
                case ValueType.Vec3:
                    return Vector3.Parse(val);
                case ValueType.Vec4:
                    return Vector4.Parse(val);
                default: return 0;
            }
        }

        public void SetValue(string name, ValueType type, object data)
        {
            string s = "null";

            switch (type)
            {
                case ValueType.Bool:
                    s = ((int)data).ToString();
                    break;
                case ValueType.Float:
                    s = (data).ToString();
                    break;
                case ValueType.Int:
                     s = data.ToString();
                    break;
                case ValueType.Vec2:
                    s = data.ToString();
                    break;
                case ValueType.Vec3:
                    s = data.ToString();
                    break;
                case ValueType.Vec4:
                    s = data.ToString();
                    break;
            }

            //Console.WriteLine(s);

            InternalCalls.MaterialSetValue(Entity.Id, type.ToString(), name, s);
        }

        public Material(Entity entity)
        {
            Entity = entity;
        }

    }
}