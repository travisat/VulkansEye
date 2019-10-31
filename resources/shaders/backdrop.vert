#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 projection;
    mat4 view;
}
ubo;

// camera is always at center of skybox
layout(location = 0) out vec3 eyeDirection; 

void main()
{
    vec2 UV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vec4 position = vec4(UV * 2.0f + -1.0f, 0.0f, 1.0f);

    //skybox is a quad that fills the screen  
    // https://gamedev.stackexchange.com/questions/60313/implementing-a-skybox-with-glsl-version-330
    // by unprojecting the mvp (ie applying the inverse backwards)
    mat4 inverseProjection = inverse(ubo.projection);
    mat4 inverseModelView = transpose(ubo.view /* * mat4(1.0) */);
    eyeDirection = vec3(inverseModelView * inverseProjection * position);

    gl_Position = position;
}