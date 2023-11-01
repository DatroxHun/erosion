#include "camera.h"

#define PI 3.14159265358979

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

void Camera::move(vec3 delta)
{
	this->pos += delta;
}

void Camera::update_polar(float da, float db)
{
	calculate_polar(da, db, 0.0);
	this->pos = radius * vec3(cos(beta) * cos(alpha), sin(beta), cos(beta) * sin(alpha));
}

void Camera::update_polar(float da, float db, float dr)
{
	calculate_polar(da, db, dr);
	this->pos = radius * vec3(cos(beta) * cos(alpha), sin(beta), cos(beta) * sin(alpha));
}

mat4 Camera::get_matrix()
{
	return perspective(radians(FOV), aspect_ratio, .01f, 100.f) * lookAt(pos, target, up);
}

void Camera::calculate_polar(float da, float db, float dr)
{
	radius = length(pos);

	beta = asin(pos.y / radius) + db;
	beta = clamp(beta, (float)(-PI * .5 + .001), (float)(PI * .5 - .001));
	alpha = atan2(pos.z, pos.x) + da;

	radius = fmin(fmax(radius + dr, 0.5f), 7.5f);
}