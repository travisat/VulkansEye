#version 450

const int numLights = 2;

layout(triangles, invocations = numLights) in;
layout(triangle_strip, max_vertices = 3) out;

struct PointLight
{
    vec3 position;
    vec3 color;
    float lumens;
};

layout(binding = 3) uniform UniformLight
{
    PointLight light[numLights];
}
uLight;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec2 inUV[];
layout(location = 2) in vec3 inNormal[];

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;

void main(void)
{

    for (int i = 0; i < gl_in.length(); i++)
    {
        gl_Layer = gl_InvocationID;
        outPosition = inPosition[i];
        outUV = inUV[i];
        outNormal = inNormal[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}