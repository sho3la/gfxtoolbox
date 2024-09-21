#version 450

in vec3 input_position;
out vec3 _entryPointOutput_uv;

// Uniforms
uniform  mat4 model_matrix;
uniform  mat4 viewproj;

void main()
{
    vec4 cs_position = vec4(v, 1.0) * (model_matrix * viewproj);
    vec3 uv = input_position + vec3(0.5);
    
    gl_Position = cs_position;
    _entryPointOutput_uv = uv;
}
