#include "core/ShaderDefaults.h"

namespace acs
{

std::string defaultDesktopVertexSource()
{
    return R"glsl(#version 330 core
layout (location = 0) in vec2 aPos;
uniform vec4 uRect;
uniform vec2 uViewport;
uniform int uObjectId;
out vec2 vUV;
flat out int vObjectId;

void main()
{
    vec2 px = aPos * uRect.zw + uRect.xy;
    vec2 ndc = px / uViewport * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);
    vUV = aPos;
    vObjectId = uObjectId;
}
)glsl";
}

std::string defaultDesktopFragmentSource()
{
    return R"glsl(#version 330 core
in vec2 vUV;
flat in int vObjectId;
out vec4 FragColor;

void main()
{
    if (vObjectId == 0)
    {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        float edge = smoothstep(0.0, 0.06, min(min(vUV.x, 1.0 - vUV.x), min(vUV.y, 1.0 - vUV.y)));
        vec3 fill = vec3(0.95, 0.55, 0.15);
        vec3 border = vec3(0.15, 0.15, 0.18);
        FragColor = vec4(mix(border, fill, edge), 1.0);
    }
}
)glsl";
}

std::string defaultMobileVertexSource()
{
    return R"glsl(#version 300 es
precision highp float;
layout (location = 0) in vec2 aPos;
uniform vec4 uRect;
uniform vec2 uViewport;
uniform int uObjectId;
out vec2 vUV;
flat out int vObjectIdOut;

void main()
{
    vec2 px = aPos * uRect.zw + uRect.xy;
    vec2 ndc = px / uViewport * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);
    vUV = aPos;
    vObjectIdOut = uObjectId;
}
)glsl";
}

std::string defaultMobileFragmentSource()
{
    return R"glsl(#version 300 es
precision highp float;
in vec2 vUV;
flat in int vObjectIdOut;
out vec4 fragColor;

void main()
{
    if (vObjectIdOut == 0)
    {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        float edge = smoothstep(0.0, 0.06, min(min(vUV.x, 1.0 - vUV.x), min(vUV.y, 1.0 - vUV.y)));
        vec3 fill = vec3(0.95, 0.55, 0.15);
        vec3 border = vec3(0.15, 0.15, 0.18);
        fragColor = vec4(mix(border, fill, edge), 1.0);
    }
}
)glsl";
}

} // namespace acs
