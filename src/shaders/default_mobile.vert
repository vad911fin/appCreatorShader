#version 300 es
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
