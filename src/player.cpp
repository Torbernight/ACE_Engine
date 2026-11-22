#include "player.hpp"
#include "core/settings.hpp"
#include "input/input.hpp"

static constexpr glm::vec3 world_up = glm::vec3(0.0,1.0,0.0);

inline static
glm::vec3 normalize2D(const glm::vec3& v)
{
    return glm::normalize(glm::vec3(v.x, 0.0f, v.z));
}

void Player::makeProj()
{
    m.proj = glm::perspective(m.FOVy, m.aspect, m.zNear, m.zFar);
    m.view_frustum.create(m.pos, {m.right, m.up, m.front}, m.zNear, m.zFar, m.FOVy, m.aspect);
}

float Player::footstep_offset() const
{
    return std::sin(M_PIf*m.footstep_freq*m.footstep_timer)*m.footstep_amp;
}

void Player::move(float dt)
{
    using namespace glm;

    InputHandler &input = InputHandler::get();

    bool forward = input.is_key_pressed('w');
    bool backward = input.is_key_pressed('s');
    bool right = input.is_key_pressed('d');
    bool left = input.is_key_pressed('a');
    bool up = input.is_key_pressed(' ');
    bool down = input.is_key_pressed(SDLK_LCTRL);
    bool sprint = input.is_key_pressed(SDLK_LSHIFT);

    m.acc = 
    (float)(forward-backward)*normalize2D(m.front)
    + 
    (float)(right-left)*normalize2D(m.right)
    + 
    (float)(up-down)*world_up;

    m.acc *= float(m.floor_collision);

    m.jump = m.acc.y > 0;

    m.moving = m.acc != vec3(0.0);

    if(!m.floor_collision || !m.moving){
        constexpr float epsilon = 1.0e-6f;
        m.footstep_amp *= 0.1f;
        if (m.footstep_amp > epsilon){
            m.footstep_timer += dt;
        } else {
            m.footstep_timer = 0.0f;
        }
    } else {
        m.footstep_timer += dt;
        m.footstep_amp = smoothstep(0.0f, 0.2f, m.footstep_timer)*0.05;
    }

    if (dot(m.acc,m.acc) == 0){
        if (m.floor_collision){
            m.vel *= 0.1f;
            return;
        }
    } else {
        m.acc = normalize(m.acc) * vec3(1000.0f, 200.0f, 1000.0f);
        m.vel.x = m.acc.x == 0 ? 0.0f : m.vel.x;
        m.vel.z = m.acc.z == 0 ? 0.0f : m.vel.z;
        if (!m.collision){
            m.acc = vec3(0.0f);
        }
    }
    m.acc.y -= 9.8;

    m.vel += m.acc*dt;
    float speed = length(vec2(m.vel.x, m.vel.z));
    float speedcap = mix(s_walk_speed, s_sprint_speed, (float)sprint);
    m.footstep_freq = speedcap;

    if (speed > speedcap){
        vec2 V = normalize(vec2(m.vel.x, m.vel.z))*speedcap;
        m.vel = vec3(V.x, m.vel.y, V.y);
    }
    m.previousPos = m.pos;

    m.pos += m.vel*dt;
    m.realVel = (m.pos - m.previousPos) / dt;
    make_AABB();
}

void Player::make_AABB()
{
    glm::vec3 bcenter = glm::vec3(m.pos.x, m.pos.y-0.845f, m.pos.z);
    glm::vec3 bbounds = glm::vec3(0.25f, 0.845f, 0.25f);
    this->m.bounding_box = AABB(bcenter-bbounds, bcenter+bbounds);
}

void Player::makeView()
{
    glm::vec3 P = m.pos;
    P.y += footstep_offset();
    m.view = glm::lookAt(P, P+m.front, world_up);
    m.view_frustum.create(P, {m.right, m.up, m.front}, m.zNear, m.zFar, m.FOVy, m.aspect);
}

glm::mat4 Player::get_transform() const
{
    return glm::mat4(
        m.right.x, m.right.y, m.right.z, 0.0f,
        m.up.x, m.up.y, m.up.z, 0.0f,
        -m.front.x, -m.front.y, -m.front.z, 0.0f,
        m.pos.x, m.pos.y, m.pos.z, 1.0f
    );
}

void Player::rotate(float dt)
{

    SettingsManager &settings = SettingsManager::get();

    float mouse_sensitivity = settings["mouse sensitivity"]->as_float();

    InputHandler &input = InputHandler::get();

    int xrel,yrel;
    input.get_mouse_delta(&xrel, &yrel);

    glm::vec2 r(xrel, -yrel);
    m.delta_rotation = r*0.01f*mouse_sensitivity;

    m.rotation += m.delta_rotation*dt;

    m.rotation.x = fmodf(m.rotation.x, 2*M_PIf);
    m.rotation.y = glm::clamp(m.rotation.y, -M_PI_2f*0.98f, M_PI_2f*0.98f);

    glm::vec2 c(glm::cos(m.rotation)), s(glm::sin(m.rotation));

    m.front.x = s.x*c.y;
    m.front.y = s.y;
    m.front.z = -c.x*c.y;

    m.front = glm::normalize(m.front);
    m.right = glm::normalize(glm::cross(m.front, world_up));
    m.up    = glm::normalize(glm::cross(m.right, m.front));
}