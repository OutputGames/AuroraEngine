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
        public static extern void MaterialGetTextures(uint id, out List<Texture> textures);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void MaterialSetUniforms(uint id, ref Dictionary<string, object> uniforms);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void MaterialGetUniforms(uint id, out Dictionary<string, object> uniforms);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public static extern void TransformSetPosition(uint id, ref Vector3 position);
    }
}