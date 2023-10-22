#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class Camera
{
public:
	vec3 pos;
	vec3 target;
	vec3 up;

	float FOV;
	float aspect_ratio;

private:
	vec3 dir;
	vec3 right;
	vec3 c_up;

public:
	Camera();
	Camera(vec3 pos, vec3 target, vec3 up, float FOV = 45.f, float aspect_ratio = 1.f);

	void set_target(vec3 target);
	void set_up(vec3 up);
	
	mat4 get_matrix();
};

#endif