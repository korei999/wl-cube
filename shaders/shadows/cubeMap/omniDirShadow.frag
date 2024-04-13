#version 320 es
precision highp float;

in VOut {
    vec3 fragPos;
    vec3 norm;
    vec2 tex;
} vIn;

uniform sampler2D uDiffuseTex;
uniform samplerCube uDepthMap;

uniform vec3 uLightPos;
uniform vec3 uViewPos;

uniform float uFarPlane;

out vec4 outColor;

vec3 sampleOffsetDirections[20] = vec3[](
    vec3( 1, 1, 1), vec3( 1,-1, 1), vec3(-1,-1, 1), vec3(-1, 1, 1),
    vec3( 1, 1,-1), vec3( 1,-1,-1), vec3(-1,-1,-1), vec3(-1, 1,-1),
    vec3( 1, 1, 0), vec3( 1,-1, 0), vec3(-1,-1, 0), vec3(-1, 1, 0),
    vec3( 1, 0, 1), vec3(-1, 0, 1), vec3( 1, 0,-1), vec3(-1, 0,-1),
    vec3( 0, 1, 1), vec3( 0,-1, 1), vec3( 0,-1,-1), vec3( 0, 1,-1)
);

float
shadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - uLightPos;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.02;
    float offset = 0.1;
    int samples = 20;
    float viewDist = length(uViewPos - fragPos);
    // float diskRadius = 0.05;
    float diskRadius = (1.0 + (viewDist / uFarPlane)) * 0.04;
    for (int i = 0; i < samples; i++)
    {
        float closestDepth = texture(uDepthMap,
                                     fragToLight +
                                     sampleOffsetDirections[i] *
                                     diskRadius).r;
        closestDepth *= uFarPlane;
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }

    return shadow * 0.05; /* shadow / 20 */

    // /* get vector between fragment position and light position */
    // vec3 fragToLight = fragPos - uLightPos;
    // /* ise the fragment to light vector to sample from the depth map     */
    // float closestDepth = texture(uDepthMap, fragToLight).r;
    // /* it is currently in linear range between [0,1], let's re-transform it back to original depth value */
    // closestDepth *= uFarPlane;
    // /* now get current linear depth as the length between the fragment and light position */
    // float currentDepth = length(fragToLight);
    // /* test for shadows */
    // float bias = 0.005; /* we use a much larger bias since depth is now in [near_plane, uFarPlane] range */
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;        
    // /* display closestDepth as debug (to visualize depth cubemap) */
    // /* FragColor = vec4(vec3(closestDepth / uFarPlane), 1.0);     */
        
    // return shadow;
}

void
main()
{
    vec3 color = texture(uDiffuseTex, vIn.tex).rgb;
    vec3 normal = normalize(vIn.norm);
    vec3 lightColor = vec3(1.0);
    /* ambient */
    vec3 ambient = 0.15 * color;
    /* diffuse */
    vec3 lightDir = normalize(uLightPos - vIn.fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    /* specular */
    vec3 viewDir = normalize(uViewPos - vIn.fragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    /* calculate shadow */
    float shadow = shadowCalculation(vIn.fragPos);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    outColor = vec4(lighting, 1.0);
}