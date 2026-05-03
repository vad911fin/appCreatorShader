#version 330 core
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
