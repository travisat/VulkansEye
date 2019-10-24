#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(binding = 1) uniform shadowVP
{
    mat4 matrix[6];
}
shadows;

layout(location = 0) in vec4 inPosition[];

layout(location = 0) out vec4 outPosition;

void main(void)
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for (int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            outPosition = inPosition[i];
            gl_Position = shadows.matrix[face] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}