#ifndef MODEL_HPP
#define MODEL_HPP

#include <map>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "rendering/shader.hpp"
#include "utils/utils.hpp"

#include "texture.hpp"

namespace Engine
{
	class Shader;
}

class AssimpGLMHelpers
{
public:

    static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
    {
        glm::mat4 to;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

    static inline glm::vec3 GetGLMVec(const aiVector3D& vec)
    {
        return glm::vec3(vec.x, vec.y, vec.z);
    }

    static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
    {
        return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
    }
};

#define MAX_BONE_INFLUENCE 4

struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
	//bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct BoneInfo
{
	/*id is index in finalBoneMatrices*/
	int id;

	/*offset matrix transforms vertex from model space to bone space*/
	glm::mat4 offset;

};

struct Material
{

	std::vector<Texture> textures;

    struct UniformData
    {
        GLenum type;
        union
        {
            bool b;
            int i;
            float f;
            vec2 v2;
            vec3 v3;
            vec4 v4;
            mat2 m2;
            mat3 m3;
            mat4 m4;
        };
    };

    std::vector<std::pair<string, UniformData*>> uniforms;

    UniformData* GetUniform(string name)
    {
	    for (std::pair<string, UniformData*> uniform : uniforms)
	    {
		    if (uniform.first == name)
		    {
                return uniform.second;
                break;
		    }
	    }
        return nullptr;
    }

    void ProcessUniforms()
    {
        GLint i;
        GLint count;

        GLint size; // size of the variable
        GLenum type; // type of the variable (float, vec3 or mat4, etc)

        const GLsizei bufSize = 16*4; // maximum name length
        GLchar name[bufSize]; // variable name in GLSL
        GLsizei length; // name length
        glGetProgramiv(shader->ID, GL_ACTIVE_UNIFORMS, &count);
        //printf("Active Uniforms: %d\n", count);

        uniforms.clear();

        for (i = 0; i < count; i++)
        {
            glGetActiveUniform(shader->ID, (GLuint)i, bufSize, &length, &size, &type, name);

            //printf("Uniform #%d Type: %u Name: %s\n", i, type, name);

            UniformData* dat = new UniformData;
            dat->type = type;

            /*
            switch (type) {
            case GL_BOOL:
                cout << name << " is bool";
                break;
            case GL_INT:
                cout << name << " is int";
                break;
            case GL_FLOAT:
                cout << name << " is float";
                break;
            case GL_FLOAT_VEC2:
                cout << name << " is vec2";
                break;
            case GL_FLOAT_VEC3:
                cout << name << " is vec3";
                break;
            case GL_FLOAT_VEC4:
                cout << name << " is vec4";
                break;
            case GL_FLOAT_MAT2:
                cout << name << " is mat2";
                break;
            case GL_FLOAT_MAT3:
                cout << name << " is mat3";
                break;
            case GL_FLOAT_MAT4:
                cout << name << " is mat4";
                break;
            }
            */

            //cout << std::endl;

            uniforms.push_back({ name, dat });
        }
    }

	Shader* shader;
};

struct Mesh
{
	unsigned int VBO, VAO, EBO;

	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		vector<Texture> textures;
        string name;
	};

	MeshData* data;

	void Draw();

	static Mesh* Upload(MeshData* data);

	Material* material;

};

struct Model
{
	std::vector<Mesh*> meshes;

	void Draw();

	static Model* LoadModel(std::string path);

    map<string, BoneInfo> m_BoneInfoMap; //
    int m_BoneCounter = 0;

    auto& GetBoneInfoMap()
    {
	    return m_BoneInfoMap;
    }
    int& GetBoneCount()
    {
	    return m_BoneCounter;
    }

    void SetShader(Shader* shader)
    {
	    for (int i = 0; i < meshes.size(); ++i)
	    {
            meshes[i]->material->shader = shader;
            meshes[i]->material->ProcessUniforms();
	    }
    }

    void SetVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }

    }

    void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.m_BoneIDs[i] < 0)
            {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }

	void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = m_BoneCounter;
                newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                    mesh->mBones[boneIndex]->mOffsetMatrix);
                m_BoneInfoMap[boneName] = newBoneInfo;
                boneID = m_BoneCounter;
                m_BoneCounter++;
            }
            else
            {
                boneID = m_BoneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }

};


#endif