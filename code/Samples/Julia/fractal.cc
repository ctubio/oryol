//------------------------------------------------------------------------------
//  fractal.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "fractal.h"
#include "Gfx/Gfx.h"

using namespace Oryol;

namespace Julia {

//------------------------------------------------------------------------------
fractal::fractal() {
    // empty
}

//------------------------------------------------------------------------------
fractal::~fractal() {
    o_assert_dbg(!this->isValid());
}

//------------------------------------------------------------------------------
void
fractal::setup(int w, int h, const glm::vec4& rect_, const glm::vec2& pos_, Id fsqMesh, const VertexLayout& fsqLayout) {
    o_assert_dbg(!this->isValid());

    this->rect = rect_;
    this->pos = pos_;
    this->frameIndex = 0;
    this->fsqMeshBlock[0] = fsqMesh;

    // generate a new resource label for our gfx resources
    this->label = Gfx::PushResourceLabel();

    // the fractal-rendering shader
    Id fractalShader = Gfx::CreateResource(Shaders::Julia::Setup());

    // create the ping-pong render target that hold the fractal state
    auto rtSetup = TextureSetup::RenderTarget(w, h);
    rtSetup.ColorFormat = PixelFormat::RGBA32F;
    rtSetup.DepthFormat = PixelFormat::None;
    rtSetup.ClearHint = ClearState::ClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    rtSetup.Sampler.MinFilter = TextureFilterMode::Nearest;
    rtSetup.Sampler.MagFilter = TextureFilterMode::Nearest;
    rtSetup.Sampler.WrapU = TextureWrapMode::MirroredRepeat;
    rtSetup.Sampler.WrapV = TextureWrapMode::MirroredRepeat;
    for (int i = 0; i < 2; i++) {
        this->fractalTexture[i] = Gfx::CreateResource(rtSetup);
    }

    // create a color render target that holds the fractal state as color texture
    rtSetup.ColorFormat = PixelFormat::RGBA8;
    rtSetup.Sampler.MinFilter = TextureFilterMode::Linear;
    rtSetup.Sampler.MagFilter = TextureFilterMode::Linear;
    this->colorTexture = Gfx::CreateResource(rtSetup);

    // create draw state for updating the fractal state
    auto ps = PipelineSetup::FromLayoutAndShader(fsqLayout, fractalShader);
    ps.RasterizerState.CullFaceEnabled = false;
    ps.DepthStencilState.DepthCmpFunc = CompareFunc::Always;
    ps.BlendState.ColorFormat = PixelFormat::RGBA32F;
    ps.BlendState.DepthFormat = PixelFormat::None;
    this->fractalPipeline = Gfx::CreateResource(ps);

    // create draw state to map fractal state into color texture
    Id colorShader = Gfx::CreateResource(Shaders::Color::Setup());
    ps.Shader = colorShader;
    ps.BlendState.ColorFormat = PixelFormat::RGBA8;
    this->colorPipeline = Gfx::CreateResource(ps);

    Gfx::PopResourceLabel();
}

//------------------------------------------------------------------------------
void
fractal::discard() {
    o_assert_dbg(this->isValid());

    Gfx::DestroyResources(this->label);
    this->label.Invalidate();
    this->colorTexture.Invalidate();
    this->fractalPipeline.Invalidate();
    for (auto& tex : this->fractalTexture) {
        tex.Invalidate();
    }
    this->colorPipeline.Invalidate();
}

//------------------------------------------------------------------------------
bool
fractal::isValid() const {
    return this->label.IsValid();
}

//------------------------------------------------------------------------------
void
fractal::update() {

    // increment frame and reset fractal state in first frame
    if (0 == this->frameIndex++) {
        const ClearState clearState = ClearState::ClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
        Gfx::ApplyRenderTarget(this->fractalTexture[0], clearState);
        Gfx::ApplyRenderTarget(this->fractalTexture[1], clearState);
    }

    // update frame index, and get the indices for write and read state
    const int32 writeIndex = this->frameIndex & 1;
    const int32 readIndex  = (this->frameIndex + 1) & 1;

    // render next fractal iteration
    this->fractalVSParams.Rect = this->rect;
    this->fractalFSParams.JuliaPos = this->pos;
    Shaders::Julia::FSTextures juliaTextures;
    juliaTextures.Texture = this->fractalTexture[readIndex];
    Gfx::ApplyRenderTarget(this->fractalTexture[writeIndex], ClearState::ClearNone());
    Gfx::ApplyDrawState(this->fractalPipeline, this->fsqMeshBlock, juliaTextures);
    Gfx::ApplyUniformBlock(this->fractalVSParams);
    Gfx::ApplyUniformBlock(this->fractalFSParams);
    Gfx::Draw(0);

    // map current fractal state to color texture
    Shaders::Color::FSTextures colorTextures;
    colorTextures.Texture = this->fractalTexture[writeIndex];
    Gfx::ApplyRenderTarget(this->colorTexture, ClearState::ClearNone());
    Gfx::ApplyDrawState(this->colorPipeline, this->fsqMeshBlock, colorTextures);
    Gfx::Draw(0);

    if (this->frameIndex >= this->cycleCount) {
        this->frameIndex = 0;
    }
}

} // namespace Julia
