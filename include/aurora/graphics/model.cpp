#include "model.hpp"

#include "engine/entity.hpp"
#include "rendering/render.hpp"
#include "rendering/renderer.hpp"

Material::Material()
{
    shader = new Shader("editor/shaders/0/");
    entity = nullptr;
}

void Material::Update()
{
    if (shader) {
        shader->reload();
        shader->use();

        std::vector<std::pair<std::string, int>> typenums;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            std::string name = textures[i].type;
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
            }
            else
            {
                typenums.push_back({ name, 0 });
                number = "0";
            }

            std::string uniform = "material." + name + number;

            if (shader) {
                shader->setInt((uniform).c_str(), i);
            }

            GLenum textureTarget = GL_TEXTURE_2D;

            if (textures[i].isCubemap)
            {
                textureTarget = GL_TEXTURE_CUBE_MAP;
            }

            glBindTexture(textureTarget, textures[i].ID);
        }
        glActiveTexture(GL_TEXTURE0);


        for (std::pair<std::string, Material::UniformData> uniform : uniforms)
        {
            GLenum type = uniform.second.type;

            switch (type) {
            case GL_BOOL:
                shader->setBool(uniform.first, uniform.second.b);
                break;
            case GL_INT:
                shader->setInt(uniform.first, uniform.second.i);
                break;
            case GL_FLOAT:
                shader->setFloat(uniform.first, uniform.second.f);
                break;
            case GL_FLOAT_VEC2:
                shader->setVec2(uniform.first, uniform.second.v2);
                break;
            case GL_FLOAT_VEC3:
                shader->setVec3(uniform.first, uniform.second.v3);
                break;
            case GL_FLOAT_VEC4:
                shader->setVec4(uniform.first, uniform.second.v4);
                break;
            case GL_FLOAT_MAT2:
                shader->setMat2(uniform.first, uniform.second.m2);
                break;
            case GL_FLOAT_MAT3:
                shader->setMat3(uniform.first, uniform.second.m3);
                break;
            case GL_FLOAT_MAT4:
                shader->setMat4(uniform.first, uniform.second.m4);
                break;
            }
        }
    }
}

void Mesh::Draw()
{
    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, data->indices.size(), GL_UNSIGNED_INT, 0);
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

	return mesh;
}

Mesh* Mesh::Load(std::string path, int meshIndex)
{
    Model* temp_mdl = Model::LoadModel(path);

    return temp_mdl->meshes[meshIndex];
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
        const std::size_t last_slash_idx = modelPath.rfind('\\');
        if (std::string::npos != last_slash_idx)
        {
            directory = modelPath.substr(0, last_slash_idx);
        }

        std::filesystem::path spath(str.C_Str());

        std::string spaths{ spath.filename().u8string() };

        path = directory +"//" + spaths;

        Logger::Log("Loading texture for model at at " + path, Logger::INFO, "MODEL");

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

Mesh* processMesh(aiMesh* mesh, const aiScene* scene, Model* model, std::string mpath,int meshIndex)
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
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_METALNESS, "texture_metallic", mpath);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", mpath);
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_SHININESS, "texture_rgh", mpath);
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    std::vector<Texture> emmMaps = loadMaterialTextures(material, aiTextureType_EMISSIVE, "texture_emm", mpath);
    textures.insert(textures.end(), emmMaps.begin(), emmMaps.end());

    std::vector<Texture> aoMaps = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, "texture_ao", mpath);
    textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

    Mesh::MeshData* data = new Mesh::MeshData();

    data->vertices = vertices;
    data->indices = indices;
    data->textures = textures;
    data->name = mesh->mName.C_Str();
    data->index = meshIndex;
    data->path = model->path;
    

    // return a mesh object created from the extracted mesh data
    return Mesh::Upload(data);
}

void Model::processNode(aiNode* node, const aiScene* scene, Model* model, std::string mpath)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        model->meshes.push_back(processMesh(mesh, scene, model, mpath,node->mMeshes[i]));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, model, mpath);
    }
}

Entity* processEntity(aiNode* node, const aiScene* scene, Model* model, Entity* parent)
{

    Entity* e = Scene::GetScene()->entity_mgr->CreateEntity(node->mName.C_Str());

    e->SetParent(parent);

    aiVector3t<float> scl;
    aiVector3t<float> rot;
    aiVector3t<float> pos;

    node->mTransformation.Decompose(scl, rot, pos);

    e->transform->position = { pos[0], pos[1], pos[3] };
    //e->transform->scale = { scl[0], scl[1], scl[2] };

    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        Mesh* m = model->meshes[node->mMeshes[i]];
        MeshRenderer* r = e->AttachComponent<MeshRenderer>();
        r->mesh = m;
    }

    e->Init();

    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processEntity(node->mChildren[i], scene, model, e);
    }

    return e;
}

Entity* Model::Load(string path)
{

    Assimp::Importer import;
    const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        Logger::Log("Failed to load model at  " + path + "for reason" + import.GetErrorString(), Logger::LOG_ERROR, "MODEL");
        return nullptr;
    }

    Model* m = LoadModel(path);

    filesystem::path p(path);

    Entity* entity = Scene::GetScene()->entity_mgr->CreateEntity(p.stem().string());

    for (int i = 0; i < scene->mRootNode->mNumChildren; ++i)
    {
        Entity* e = processEntity (scene->mRootNode->mChildren[i], scene, m, entity);

    }

    return entity;
}

Model* Model::LoadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        Logger::Log("Failed to load model at  " + path + "for reason" + import.GetErrorString(), Logger::LOG_ERROR, "MODEL");
        return nullptr;
    }
    //directory = path.substr(0, path.find_last_of('/'));

    Model* model = new Model();

    if (scene->mNumMeshes > 0)
    {

        aiAABB abb = scene->mMeshes[0]->mAABB;
        aiVector3D min = abb.mMin;
        aiVector3D max = abb.mMax;

        model->abmin = { min.x, min.y, min.z };
        model->abmax = { max.x, max.y, max.z };
            
    }

    model->path = path;

    processNode(scene->mRootNode, scene, model, path);

    return model;
}

unsigned Model::GetIcon()
{
    if (iconID == 0 && meshes.size() > 0)
    {

        //glDeleteTextures(1, &iconID);

        static Shader* s = nullptr;

        if (s == nullptr)
        {
            s = new Shader("editor/shaders/8");
        } else
        {
            s->reload();
        }

        SetShader(s);

        s->use();

        vec3 eye = { 10,10,10 };
        vec3 center = { 0,0,0 };
        vec3 up = { 0,1,0 };

        mat4 v = lookAt(eye, center, up);

        const float radius = 10.0f;
        float camX = sin(glfwGetTime()) * radius;
        float camZ = cos(glfwGetTime()) * radius;
        glm::mat4 view;

        vec3 abbcenter = (vec3{ abmin + abmax }) * 0.5f;

        vec3 objsize = abmin - abmax;

        float sz = std::max(objsize.x, objsize.y);

        float fov = 22.5f;

        float cv = 2.0f * tan(0.5f * DEG2RAD * fov);

        float camdist = 2.0f;

        float dist = camdist * sz / cv;

        dist += 0.5f * sz;

        vec3 pos = abbcenter - dist * vec3{ 0,0,1 };

        vec3 off = { 1,-0.25,0};

        off *= dist;

        view = glm::lookAt(pos+off, glm::vec3(0,0, 0), glm::vec3(0.0, 1.0, 0.0));


        mat4 p = glm::perspective(glm::radians(fov), 1.0f, 0.01f, 1000.0f);

        s->setMat4("view", view);
        s->setMat4("projection", p);

        mat4 m(1.0);

        m = scale(m, vec3{ 1.0f });

        s->setMat4("model", m);

        if (buffer == nullptr)
        {
            buffer = new TextureColorBuffer;
        }

        buffer->Resize({ 500,500 });

        glViewport(0, 0, 500,500);
        buffer->Bind();

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_CULL_FACE);

        Draw();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        iconID = buffer->texture;

        //buffer->Unload();
    }
    return iconID;
}
