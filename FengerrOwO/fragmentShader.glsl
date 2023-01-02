#version 460 core

out vec4 color;

in vec3 v_normal;
in vec3 v_position;

in vec3 fragmentColor;

uniform vec3 LightPosition;
uniform vec3 LightDirection;


uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;

void main(){
    
  vec3 light_direction = normalize(LightPosition - v_position);

    if (dot(light_direction, v_normal) < 0.0) {
		  light_direction = normalize(v_position - LightPosition);
	}

    vec3 camera_dir = normalize(-v_position);

    vec3 half_direction = normalize(light_direction + camera_dir);

    vec3 normal = normalize(v_normal);

    float diffuse = max(dot(normal, light_direction), 0.0);

    float specular = pow(max(dot(normal, half_direction), 0.0), 32.0);

    color = vec4((ambient_color + diffuse * diffuse_color + specular * specular_color) * fragmentColor, 1.0);
}