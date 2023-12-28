#include "camera.hpp"

#include "graphics/billboard.hpp"
#include "utils/math.hpp"

CLASS_DEFINITION(Component, Camera)

static Camera* Main;

mat4 CameraBase::GetViewMatrix()
{

    vec3 dir = vec3{ 0.001 };

    const float radius = 10.0f;
    float camX = sin(direction.y) * radius;
    float camZ = cos(direction.y) * radius;

    //dir.x = camX;
    //dir.z = camZ;

    dir += direction;
        

    //dir.x = cos(radians(dir.y)) * cos(radians(dir.x));
    //dir.y = sin(radians(dir.x));
    //dir.z = sin(radians(dir.y)) * cos(radians(dir.y));

    if (isnan(position.x))
    {
        position = vec3{ 0,1,5 };
    }

    if (isnan(direction.x))
    {
        direction = vec3{ 0 };
    }

    if (isnan(lookat.x))
    {
        lookat = vec3{ 0,0,1 };
    }

    if (isnan(up.x))
    {
        up = vec3{ 0,1,0 };
    }

    if (isnan(right.x))
    {
        right = vec3{ 1,0,0 };
    }

    vec3 center = position + dir;

    if (!useDirection)
    {
        center = lookat;
    }

    mat4 view = glm::lookAt(position, center, up);


    //glm::mat4 view;
    //view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), up);

    return view;
}

void CameraBase::Update(bool igv)
{







    framebuffer->Render(igv);

}

CameraBase* CameraBase::CreateCamera()
{

    CameraBase* cb = new CameraBase;

    vec2 size = RenderMgr::GetSceneWinSize();

    cb->framebuffer = RenderTexture::Load(size.x, size.y, cb);

    return cb;
}

void CameraBase::CopyView(mat4 view)
{
    mat4 realview = inverse(view);

    up = realview[1];
    right = realview[0];
    position = realview[3];
}

void Camera::LateUpdate()
{
    m_Cam->useDirection = true;
    m_Cam->position = entity->transform->position;

    vec3 d = Math::FixEulers(entity->transform->GetEulerAngles());

    float yaw = d.y;
    float pitch = d.x;

    d.x = cos((yaw)) * cos((pitch));
    d.y = sin((pitch));
    d.z = sin((yaw)) * cos((pitch));

    m_Cam->direction = d;
    m_Cam->up = vec3{ 0,1,0 };
    m_Cam->FOV = FieldOfView;

    m_Cam->Update(true);

    mat4 v = m_Cam->GetViewMatrix();

    //m_Cam->CopyView(v);

    entity->GetComponent<Billboard>()->color = vec3{ 1 };

    if (Main == this)
    {
        isMain = true;
    }
}

void Camera::Init()
{
    m_Cam = CameraBase::CreateCamera();
    m_Cam->useDirection = true;
    if (Main == nullptr)
    {
        Main = this;
        isMain = true;
    }

    if (!entity->GetComponent<Billboard>()) {
        Billboard* billboard = entity->AttachComponent<Billboard>();

        billboard->texture = Texture::Load("Assets/Editor/icons/svgs/solid/camera.png", false);

        billboard->Init();
    }
    else
    {
        Billboard* billboard = entity->GetComponent<Billboard>();

        billboard->texture = Texture::Load("Assets/Editor/icons/svgs/solid/camera.png", false);
    }
}

void Camera::EngineRender()
{

    if (ImGui::Checkbox("Main", &isMain))
    {
	    if (isMain)
	    {
            Main = this;
	    } else
	    {
            Main = nullptr;
	    }
    }

    ImGui::DragFloat("FOV", &FieldOfView, 1);

    

}

Camera* Camera::GetMain()
{
    return Main;
}
