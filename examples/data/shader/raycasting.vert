#version 400


layout (location = 0) in vec3 VerPos;


out vec3 EntryPoint;
out vec4 ExitPointCoord;

uniform mat4 MVP;

void main()
{
    EntryPoint = VerPos;
    gl_Position = MVP * vec4(VerPos,1.0);
}
