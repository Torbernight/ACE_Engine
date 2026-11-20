#pragma once 

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../geometry/frustum.hpp"

namespace Renderer 
{

class Camera 
{
    public:

    struct CameraShaderParameters
    {
        glm::vec4 pos;
        glm::vec4 front;
        float zNear;
        float zFar;
    };


    const glm::mat4& get_projectionMatrix() const {
        return projectionMatrix;
    };
    const glm::mat4& get_viewMatrix() const {
        return viewMatrix;
    };
    const glm::vec3& get_position() const {
        return position;
    };
    const glm::vec3& get_front() const {
        return front;
    };
    const glm::vec3& get_right() const {
        return right;
    };
    const glm::vec3& get_up() const {
        return up;
    };
    const glm::vec2& get_viewport_size() const {
        return viewportSize;
    };
    float get_FOV() const {
        return FOV;
    }
    float get_near_plane() const {
        return nearPlane;
    }
    float get_far_plane() const {
        return farPlane;
    }
    float get_aspect_ratio() const {
        return aspectRatio;
    }


    void set_position(const glm::vec3& v){
        position = v;
        update_view_matrix();
        update_view_frustum();
    };

    void set_basis(const glm::vec3& right, const glm::vec3& up, const glm::vec3 &front){
        this->front = front;
        this->right = right;
        this->up = up;
        update_view_matrix();
        update_view_frustum();
    }

    void set_basis(const glm::vec3& up, const glm::vec3 &front){
        this->front = front;
        this->up = up;
        this->right = glm::normalize(glm::cross(front, up));
        update_view_matrix();
        update_view_frustum();
    }

    void set_front(const glm::vec3& v){
        front = v;
        this->right = glm::normalize(glm::cross(front, up));
        update_view_matrix();
        update_view_frustum();
    };

    void set_up(const glm::vec3& v){
        up = v;
        this->right = glm::normalize(glm::cross(front, up));
        update_view_frustum();
    };
    void set_viewport_size(const glm::vec2& v){
        viewportSize = v;
        aspectRatio = v.x/v.y;
        update_projection_matrix();
        update_view_frustum();
    };

    void set_FOV(float v) {
        FOV = v;
        update_projection_matrix();
        update_view_frustum();
    }
    void set_near_plane(float v) {
        nearPlane = v;
        update_projection_matrix();
        update_view_frustum();
    }
    void set_far_plane(float v) {
        farPlane = v;
        update_projection_matrix();
        update_view_frustum();
    }
    void set_aspect_ratio(float v) {
        aspectRatio = v;
        update_projection_matrix();
        update_view_frustum();
    }
    
    private:

    void update_projection_matrix()
    {
        projectionMatrix = glm::perspective(FOV, aspectRatio, nearPlane, farPlane);
    }

    void update_view_matrix()
    {
        static constexpr glm::vec3 world_up = glm::vec3(0.0,1.0,0.0);
        viewMatrix = glm::lookAt(position, position+front, world_up);
    }

    void update_view_frustum()
    {
        viewFrustum.create(position, {right, up, front}, nearPlane, farPlane, FOV, aspectRatio);
    }

    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    frustum viewFrustum;
    glm::vec3 position;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 front;
    glm::vec2 viewportSize;
    float aspectRatio;
    float FOV;
    float nearPlane;
    float farPlane;


};



}
