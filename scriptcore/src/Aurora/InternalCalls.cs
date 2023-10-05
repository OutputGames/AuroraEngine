using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Aurora {
    public static class InternalCalls

    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern Entity CreateEntity(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern Entity GetEntity(uint id);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern bool EntityHasComponent(uint id,  Type componentType);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void TransformGetPosition(uint id,  out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern string MaterialGetValue(uint id, string type, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void MaterialSetValue(uint id, string type, string name, string data);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void MaterialGetTextures(uint id, out List<Texture> textures);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void TransformSetPosition(uint id, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern float TimeGetTime();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern float TimeGetSineTime();
    }
}