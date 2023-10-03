
using System.Collections.Generic;

namespace Aurora
{
    public class Mesh {

        public MeshData data;
    }

    public struct MeshData
    {
        public List<Vertex> Vertices;
        public List<uint> indices;
        public string name;
        public string path;
        public int index;
    }

    public struct Vertex
    {
        public Vector3 Position;
        public Vector3 Normal;
        public Vector2 TexCoords;
        public Vector3 Tangent;
        public Vector3 Bitangent;
    }

}