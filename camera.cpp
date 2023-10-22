#include "camera.h"

Camera::Camera()
{
	this->pos = vec3(.0, .0, .1);
	this->set_target(vec3(0.));
	this->set_up(vec3(0., 1., 0.));

	FOV = 45.;
	aspect_ratio = 1.0;
}

Camera::Camera(vec3 pos, vec3 target, vec3 up, float FOV, float aspect_ratio)
{
	this->pos = pos;
	this->set_target(target);
	this->set_up(up);

	this->FOV = FOV;
	this->aspect_ratio = aspect_ratio;
}

void Camera::set_target(vec3 target)
{
	this->target = target;
	this->dir = normalize(target - pos);
}

void Camera::set_up(vec3 up)
{
	this->up = up;
	this->right = normalize(cross(up, dir));
	this->c_up = cross(dir, right);
}

mat4 Camera::get_matrix()
{
	return perspective(radians(this->FOV), this->aspect_ratio, .01f, 100.f) * lookAt(this->pos, this->target, this->up);
}