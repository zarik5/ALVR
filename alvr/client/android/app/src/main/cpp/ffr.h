#pragma once

#include <memory>
#include <vector>

#include "gl_render_utils/render_pipeline.h"
#include "bindings.h"

struct FFRData {
    bool enabled;
    uint32_t eyeWidth;
    uint32_t eyeHeight;
    EyeFov leftEyeFov;
    float foveationStrength;
    float foveationShape;
    float foveationVerticalOffset;
};

class FFR {
public:
    FFR(gl_render_utils::Texture *inputSurface);

    void Initialize(FFRData ffrData);

    void Render() const;

    gl_render_utils::Texture *GetOutputTexture() { return mExpandedTexture.get(); }

private:

    gl_render_utils::Texture *mInputSurface;
    std::unique_ptr<gl_render_utils::Texture> mExpandedTexture;
    std::unique_ptr<gl_render_utils::RenderState> mExpandedTextureState;
    std::unique_ptr<gl_render_utils::RenderPipeline> mDecompressSlicesPipeline;
};
