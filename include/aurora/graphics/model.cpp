#include "model.hpp"

#include "rendering/render.hpp"

void Mesh::Draw()
{
    std::vector<std::pair<std::string, int>> typenums;
    for (unsigned int i = 0; i < material->textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = material->textures[i].type;
        bool couldFind = false;
        if (typenums.size() > 0) {
            int ctr = 0;
            for (std::pair<std::string, int> typenum : typenums)
            {
                if (typenum.first == name)
                {
                    typenums[ctr].second += 1;
                    number = to_string(typenums[ctr].second);
                    couldFind = true;
                }
                ctr++;
            }
            if (couldFind == false)
            {
                typenums.push_back({ name, 0 });
                number = "0";
            }
        } else
        {
            typenums.push_back({ name, 0 });
            number = "0";
        }

        std::string uniform = "material." + name + number;

        material->shader->setInt((uniform).c_str(), i);

        GLenum textureTarget = GL_TEXTURE_2D;

        if (material->textures[i].isCubemap)
        {
            textureTarget = GL_TEXTURE_CUBE_MAP;
        }

        glBindTexture(textureTarget, material->textures[i].ID);
    }
    glActiveTexture(GL_TEXTURE0);

    for (std::pair<std::string, Material::UniformData*> uniform : material->uniforms)
    {
        GLenum type = uniform.second->type;

        switch (type) {
        case GL_BOOL:
            material->shader->setBool(uniform.first,uniform.second->b);
            break;
        case GL_INT:
            material->shader->setInt(uniform.first, uniform.second->i);
            break;
        case GL_FLOAT:
            material->shader->setFloat(uniform.first, uniform.second->f);
            break;
        case GL_FLOAT_VEC2:
            material->shader->setVec2(uniform.first, uniform.second->v2);
            break;
        case GL_FLOAT_VEC3:
            material->shader->setVec3(uniform.first, uniform.second->v3);
            break;
        case GL_FLOAT_VEC4:
            material->shader->setVec4(uniform.first, uniform.second->v4);
            break;
        case GL_FLOAT_MAT2:
            material->shader->setMat2(uniform.first, uniform.second->m2);
            break;
        case GL_FLOAT_MAT3:
            material->shader->setMat3(uniform.first, uniform.second->m3);
            break;
        case GL_FLOAT_MAT4:
            material->shader->setMat4(uniform.first, uniform.second->m4);
            break;
        }
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, data->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Mesh* Mesh::Upload(MeshData* data)
{
	Mesh* mesh = new Mesh();

    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

    glBufferData(GL_ARRAY_BUFFER, data->vertices.size() * sizeof(Vertex), &data->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data->indices.size() * sizeof(unsigned int),
        &data->indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    // ids
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, m_Weights));

    glBindVertexArray(0);

    mesh->data = data;

    mesh->material = new Material();

    mesh->material->textures = data->textures;

	return mesh;
}

Mesh* Mesh::Load(std::string path)
{
    Model* temp_mdl = Model::LoadModel(path);

    return temp_mdl->meshes[0];
}

void Model::Draw()
{
	for (int i = 0; i < meshes.size(); ++i)
	{
        meshes[i]->Draw();
	}
}

std::vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, std::string modelPath)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        std::string directory;
        std::string path;
        const std::size_t last_slash_idx = modelPath.rfind('/');
        if (std::string::npos != last_slash_idx)
        {
            directory = modelPath.substr(0, last_slash_idx);
        }

        std::filesystem::path spath(str.C_Str());

        std::string spaths{ spath.filename().u8string() };

        path = directory +"//" + spaths;

        std::cout << "loading texture for model at " << path << std::endl;

        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (strcmp(textures_loaded[j].path.data(), path.c_str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture = Texture::Load(path);
            texture.type = typeName;
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
        }
    }
    return textures;
}

Mesh* processMesh(aiMesh* mesh, const aiScene* scene, Model* model, std::string mpath)
{
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        

        glm::vec3 vector; // we declare a placeholder std::vector since assimp uses its own std::vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;

            if (mesh->HasTangentsAndBitangents()) {
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    model->ExtractBoneWeightForVertices(vertices, mesh, scene);

    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices std::vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", mpath);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", mpath);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", mpath);
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_SHININESS, "texture_tcl", mpath);
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    std::vector<Texture> emmMaps = loadMaterialTextures(material, aiTextureType_EMISSIVE, "texture_emm", mpath);
    textures.insert(textures.end(), emmMaps.begin(), emmMaps.end());

    Mesh::MeshData* data = new Mesh::MeshData();

    data->vertices = vertices;
    data->indices = indices;
    data->textures = textures;
    data->name = mesh->mName.C_Str();
    

    // return a mesh object created from the extracted mesh data
    return Mesh::Upload(data);
}

void processNode(aiNode* node, const aiScene* scene, Model* model, std::string mpath)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        model->meshes.push_back(processMesh(mesh, scene, model, mpath));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, model, mpath);
    }
}

Model* Model::LoadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return nullptr;
    }
    //directory = path.substr(0, path.find_last_of('/'));

    Model* model = new Model();

    model->path = path;

    processNode(scene->mRootNode, scene, model, path);

    return model;
}
