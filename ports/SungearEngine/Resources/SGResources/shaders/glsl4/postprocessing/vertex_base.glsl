#sg_pragma once

#sg_include "../primitives.glsl"

SGSubPass(PostProcessLayerDepthPass, PostProcessLayerFXPass, PostProcessAttachmentsCombiningPass, PostProcessFinalFXPass)
{
    SGSubShader(Vertex)
    {
        out vec2 vs_UVAttribute;

        void main()
        {
            vec2 pos = quad2DVerticesPositions[gl_VertexID].xy;

            vs_UVAttribute = quad2DUVs[gl_VertexID];

            gl_Position = vec4(pos, 0.0, 1.0);
        }
    }
}
