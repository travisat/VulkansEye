#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(binding = 0) uniform shadowmvp
{
    mat4 model;
    mat4 view[6];
    mat4 projection;
    vec4 lightpos;
}
shadows;

layout(location = 0) in vec4 inPosition[];

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 lightpos;

void main(void)
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for (int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            lightpos =  shadows.lightpos;
            outPosition = inPosition[i];
            gl_Position = shadows.projection * shadows.view[face] * shadows.model * outPosition;
            EmitVertex();
        }
        EndPrimitive();
    }
}