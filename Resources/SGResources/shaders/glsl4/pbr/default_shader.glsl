// TODO: make optimization

/**in int gl_FrontFacing;
in vec4 gl_FragCoord;*/

layout(std140) uniform ObjectMatrices
{
    mat4 objectModelMatrix;
};

layout(std140) uniform ViewMatrices
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewDirection;
};

layout(std140) uniform ProgramData
{
    vec2 windowSize;
};

struct ShadowsCaster
{
    mat4 shadowsCasterSpace;
    vec3 position;
};

struct DirectionalLight
{
    vec3 position;
    vec4 color;
};

float ambient = 0.1;

#ifdef SHADOWS_CASTERS_NUM
    uniform ShadowsCaster shadowsCasters[SHADOWS_CASTERS_NUM];
    uniform sampler2D shadowsCastersShadowMaps[SHADOWS_CASTERS_NUM];
#endif

#ifdef DIRECTIONAL_LIGHTS_NUM
    uniform DirectionalLight directionalLights[DIRECTIONAL_LIGHTS_NUM];
#endif

#ifdef VERTEX_SHADER
    layout (location = 0) in vec3 positionsAttribute;
    layout (location = 1) in vec3 UVAttribute;
    layout (location = 2) in vec3 normalsAttribute;
    layout (location = 3) in vec3 tangentsAttribute;
    layout (location = 4) in vec3 bitangentsAttribute;

    out VSOut
    {
        vec2 UV;
        vec3 normal;

        vec3 fragPos;
        mat3 TBN;
    } vsOut;

    void main()
    {
        vsOut.UV = UVAttribute.xy;
        vsOut.normal = normalsAttribute;

        //if(!gl_FrontFacing) vsOut.normal *= -1.0;

        vsOut.fragPos = vec3(objectModelMatrix * vec4(positionsAttribute, 1.0));

        vec3 T = normalize(vec3(objectModelMatrix * vec4(tangentsAttribute,   0.0)));
        vec3 B = normalize(vec3(objectModelMatrix * vec4(bitangentsAttribute, 0.0)));
        vec3 N = normalize(vec3(objectModelMatrix * vec4(normalsAttribute,    0.0)));
        vsOut.TBN = mat3(T, B, N);

        gl_Position = projectionMatrix * viewMatrix * vec4(vsOut.fragPos, 1.0);
    }
#endif

#ifdef FRAGMENT_SHADER
    float saturate(float a) { return clamp(a, 0.0, 1.0); }
    vec3 saturate(vec3 a) { return clamp(a, vec3(0.0), vec3(1.0)); }

    vec3 ACESFilm(vec3 x)
    {
        float a = 2.51f;
        float b = 0.03f;
        float c = 2.43f;
        float d = 0.59f;
        float e = 0.14f;
        return saturate((x*(a*x+b))/(x*(c*x+d)+e));
    }

    // shadows impl
    #if defined(SHADOWS_CASTERS_NUM) && defined(sgmat_shadowMap_MAX_TEXTURES_NUM)
        const vec2 poissonDisk4[4] = vec2[] (
            vec2( -0.94201624, -0.39906216 ),
            vec2( 0.94558609, -0.76890725 ),
            vec2( -0.094184101, -0.92938870 ),
            vec2( 0.34495938, 0.29387760 )
        );

        const vec2 poissonDisk16[16] = vec2[] (
            vec2( -0.94201624, -0.39906216 ),
            vec2( 0.94558609, -0.76890725 ),
            vec2( -0.094184101, -0.92938870 ),
            vec2( 0.34495938, 0.29387760 ),
            vec2( -0.91588581, 0.45771432 ),
            vec2( -0.81544232, -0.87912464 ),
            vec2( -0.38277543, 0.27676845 ),
            vec2( 0.97484398, 0.75648379 ),
            vec2( 0.44323325, -0.97511554 ),
            vec2( 0.53742981, -0.47373420 ),
            vec2( -0.26496911, -0.41893023 ),
            vec2( 0.79197514, 0.19090188 ),
            vec2( -0.24188840, 0.99706507 ),
            vec2( -0.81409955, 0.91437590 ),
            vec2( 0.19984126, 0.78641367 ),
            vec2( 0.14383161, -0.14100790 )
        );

    float random(vec2 uv)
    {
        return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123);
    }

    const float shadowsBias = 0.00003;

    float sampleShadowMap(
        const in vec2 coords,
        const in int shadowsCasterIdx,
        const in float compare,
        const in float lightMinCoeff
    )
    {
        return texture2D(shadowsCastersShadowMaps[shadowsCasterIdx], coords).z < compare ? lightMinCoeff : 1.0;
        //return step(compare, texture2D(shadowsCastersShadowMaps[shadowsCasterIdx], coords).z);
    }

    // PCF FUNCTIONS ------------------------------

    float calculateLinearDepth(
        const in vec3 projCoords,
        const in int shadowsCasterIdx,
        const in vec2 texelSize,
        const in float lightMinCoeff
    )
    {
        float finalProjZ = projCoords.z - shadowsBias;

        float shadow = 0.0;

        vec2 pixelPos = projCoords.xy / texelSize + 0.5;
        vec2 fractPart = fract(pixelPos);
        vec2 startTexel = (pixelPos - fractPart) * texelSize;

        float blTexel = sampleShadowMap(startTexel, shadowsCasterIdx, finalProjZ, lightMinCoeff);
        float brTexel = sampleShadowMap(startTexel + vec2(texelSize.x, 0.0), shadowsCasterIdx, finalProjZ, lightMinCoeff);
        float tlTexel = sampleShadowMap(startTexel + vec2(0.0, texelSize.y), shadowsCasterIdx, finalProjZ, lightMinCoeff);
        float trTexel = sampleShadowMap(startTexel + texelSize, shadowsCasterIdx, finalProjZ, lightMinCoeff);

        float mixA = mix(blTexel, tlTexel, fractPart.y);
        float mixB = mix(brTexel, trTexel, fractPart.y);

        return mix(mixA, mixB, fractPart.x);
    }

    float calculatePCF(
        const in vec3 projCoords,
        const in int shadowsCasterIdx,
        const in float lightMinCoeff,
        const in vec2 texelSize
    )
    {
        const float samplesNum = 4.0;
        const float samplesStart = (samplesNum - 1.0) / 2.0;
        const float samplesNumSquared = samplesNum * samplesNum;

        float resShadow = 0.0;

        for(float y = -samplesStart; y <= samplesStart; y += 1.0)
        {
            for(float x = -samplesStart; x <= samplesStart; x += 1.0)
            {
                vec2 offset = vec2(x, y) * texelSize;
                resShadow += calculateLinearDepth(vec3(projCoords.xy + offset, projCoords.z), shadowsCasterIdx, texelSize, lightMinCoeff);
            }
        }

        return resShadow / samplesNumSquared;
    }

    // -------------------------------------------------
    // PCSS FUNCTIONS ---------------------------------

    float calculatePCSS(
        const in vec3 normal,
        const in vec4 shadowsCasterSpaceFragPos,
        const in vec3 projCoords,
        const in int shadowsCasterIdx,
        const in vec2 texelSize,
        const in float lightMinCoeff
    )
    {
        const int numBlockerSamples = 16;
        const float nearPlane = 0.1;
        const float lightWorldSize = 0.5;
        const float lightFrustumWidth = 3.75;
        const float lightSizeUV = lightWorldSize / lightFrustumWidth;

        float searchWidth = lightSizeUV * (projCoords.z - nearPlane) / projCoords.z;

        float blockers = 0.0;
        float avgBlocker = 0.0;

        float samplingsTotal = 0.0;

        float shadow = 0.0;

        for (int j = 0; j < numBlockerSamples; ++j) {
           /** random angle
            float ang = random(vec2(gl_FragCoord)) * 2.0 - 1.0;
            float s = sqrt(1.0 - ang * ang);
            mat2 rot = mat2(ang, s, -s, ang);

            vec2 pcf_offset = poissonDisk16[j] * rot;
            vec2 uv = projCoords.xy + pcf_offset * texelSize;*/

            float occluder = texture(shadowsCastersShadowMaps[shadowsCasterIdx], projCoords.xy + poissonDisk16[j] * searchWidth).z;

            if(occluder < projCoords.z - shadowsBias)
            {
                blockers += 1.0;
                avgBlocker += occluder;
            }
        }

        avgBlocker /= blockers;

        if(blockers < 1.0)
        {
            return 1.0;
        }

        float penumbraSize = (projCoords.z - avgBlocker) / avgBlocker;
        float filterRadiusUV = penumbraSize * lightSizeUV * nearPlane / projCoords.z;

        for(int i = 0; i < 16; ++i)
        {
            vec2 offset = poissonDisk16[i].xy * filterRadiusUV;
            shadow += calculateLinearDepth(vec3(projCoords.xy + offset, projCoords.z), shadowsCasterIdx, texelSize, lightMinCoeff);
        }

        return (shadow / 16.0);
    }

    // --------------------------------------------------

    float calculateShadow(
        const in vec4 shadowsCasterSpaceFragPos,
        const in vec3 fragPos,
                 vec3 normal,
        const in int shadowsCasterIdx
    )
    {
        vec3 projCoords = shadowsCasterSpaceFragPos.xyz / shadowsCasterSpaceFragPos.w;
        projCoords = projCoords * 0.5 + 0.5;

        if(projCoords.z > 1.0)
        {
            return 1.0;
        }

        if(!gl_FrontFacing) normal *= -1;

        vec3 shadowCasterDir = normalize(shadowsCasters[shadowsCasterIdx].position -
                                         fragPos);
        float shadowFactor = dot(normal, shadowCasterDir);

        if(shadowFactor < 0.0)
        {
            return 1.0;
        }

        vec2 texelSize = 1.0 / textureSize(shadowsCastersShadowMaps[shadowsCasterIdx], 0);

        // PCF ------------------

        /**float pcfShadow = calculatePCF(projCoords, shadowsCasterIdx, 0.35, texelSize);

        return pcfShadow;
*/
        // -----------------------

        // PCSS ------------------

        float pcssShadow = calculatePCSS(normal, shadowsCasterSpaceFragPos, projCoords, shadowsCasterIdx, texelSize, 0.35);

        return pcssShadow;

        // -----------------------

        // if defined poisson disk ----------
        /*float shadowVisibility = 1.0;

        for(int i = 0; i < 4; i++)
        {
            // todo: make random poisson
            float depthFactor = texture(shadowsCastersShadowMaps[shadowsCasterIdx],
                                        projCoords.xy + poissonDisk[i] / 3000.0).z;
            depthFactor = depthFactor < projCoords.z - shadowsBias ?
            depthFactor : 0.0;

            shadowVisibility -= 0.2 * depthFactor;
        }

        return shadowVisibility / 1.5;*/
        // ------------------------
    }
    #endif

    #ifdef DIRECTIONAL_LIGHTS_NUM
        void calculateDiffuseAndSpecularColor(
            const in vec3 normal,
            const in vec3 lightPos,
            const in vec3 fragPos,
            const in vec4 lightColor,
            out vec3 diffuseColor,
            out vec3 specularColor
        )
    {
        vec3 lightDir = normalize(lightPos - fragPos);
        float finalDiffuse = max(dot(normal, lightDir), 0.0) * 6.0;

        diffuseColor = vec3(finalDiffuse) * lightColor.rgb;

        vec3 viewDir = normalize(viewDirection - fragPos);
        vec3 reflectDir = reflect(-lightDir, normal);

        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);

        specularColor = spec * lightColor.rgb;
    }
    #endif

    out vec4 fragColor;

    uniform sampler2D sgmat_emissive0;
    uniform sampler2D sgmat_ambientOcclusion1;
    uniform sampler2D sgmat_ambient2;
    uniform sampler2D sgmat_diffuseRoughness3;
    uniform sampler2D sgmat_diffuse4;
    uniform sampler2D sgmat_displacement5;
    uniform sampler2D sgmat_height6;
    uniform sampler2D sgmat_normals7;
    uniform sampler2D sgmat_baseColor8;
    uniform sampler2D sgmat_clearcoat9;
    uniform sampler2D sgmat_emissionColor10;
    uniform sampler2D sgmat_lightmap11;
    uniform sampler2D sgmat_metalness12;
    uniform sampler2D sgmat_normalCamera13;
    uniform sampler2D sgmat_opacity14;
    uniform sampler2D sgmat_relfection15;
    uniform sampler2D sgmat_sheen16;
    uniform sampler2D sgmat_shininess17;
    uniform sampler2D sgmat_specular18;
    uniform sampler2D sgmat_transmission19;

    in VSOut
    {
        vec2 UV;
        vec3 normal;

        vec3 fragPos;
        mat3 TBN;
    } vsIn;

    void main()
    {
        vec3 normalizedNormal = normalize(vsIn.normal);

        vec4 colorFromBase = vec4(1);
        vec4 colorFromDiffuse = vec4(1);
        vec4 colorFromMetalness = vec4(1);
        vec4 colorFromRoughness = vec4(1);
        vec3 colorFromNormalMap = vec3(normalizedNormal);

        vec2 finalUV = vsIn.UV;
        #ifdef FLIP_TEXTURES_Y
            finalUV.y = 1.0 - vsIn.UV.y;
        #endif

        #ifdef sgmat_metalness12_DEFINED
            colorFromMetalness = texture(sgmat_metalness12, vec2(finalUV.x, finalUV.y));
        #endif

        #ifdef sgmat_baseColor8_DEFINED
            colorFromBase = texture(sgmat_baseColor8, vec2(finalUV.x, finalUV.y));
        #endif

        #ifdef sgmat_diffuse4_DEFINED
            colorFromDiffuse = texture(sgmat_diffuse4, vec2(finalUV.x, finalUV.y));
        #endif

        #ifdef sgmat_normals7_DEFINED
            colorFromNormalMap = texture(sgmat_normals7, vec2(finalUV.x, finalUV.y)).rgb;
            colorFromNormalMap = normalize(colorFromNormalMap * 2.0 - 1.0);
            colorFromNormalMap = normalize(vsIn.TBN * colorFromNormalMap);
        #endif

        fragColor = colorFromBase;

        // todo: fix multiple directional lights
        #ifdef DIRECTIONAL_LIGHTS_NUM
            vec3 finalDiffuse = vec3(0.0);
            vec3 finalSpecular = vec3(0.0);

            vec3 intermediateDiffuse = vec3(0.0);
            vec3 intermediateSpecular = vec3(0.0);

            for(int i = 0; i < DIRECTIONAL_LIGHTS_NUM; i++)
            {
                calculateDiffuseAndSpecularColor(
                    colorFromNormalMap,
                    directionalLights[i].position,
                    vsIn.fragPos,
                    directionalLights[i].color,
                    intermediateDiffuse,
                    intermediateSpecular
                );

                finalDiffuse += intermediateDiffuse;
                finalSpecular += intermediateSpecular;
            }

            fragColor.rgb *= vec3(ambient) + (finalDiffuse * colorFromDiffuse.rgb) + finalSpecular;
        #endif

        #if defined(SHADOWS_CASTERS_NUM) && defined(sgmat_shadowMap_MAX_TEXTURES_NUM)
            float shadowCoeff = 0.0;

            for(int i = 0; i < SHADOWS_CASTERS_NUM && i < sgmat_shadowMap_MAX_TEXTURES_NUM; i += 1)
            {
                fragColor.rgb *= calculateShadow(
                    shadowsCasters[i].shadowsCasterSpace * vec4(vsIn.fragPos, 1.0),
                    vsIn.fragPos,
                    colorFromNormalMap,
                    i
                );
            }
        #endif

        fragColor.rgb = ACESFilm(fragColor.rgb);
    }
#endif
