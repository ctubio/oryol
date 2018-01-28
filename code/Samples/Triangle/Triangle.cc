//------------------------------------------------------------------------------
//  Triangle.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "shaders.h"

using namespace Oryol;

class TriangleApp : public App {
public:
    AppState::Code OnRunning();
    AppState::Code OnInit();
    AppState::Code OnCleanup();

    DrawState drawState;
};
OryolMain(TriangleApp);

//------------------------------------------------------------------------------
AppState::Code
TriangleApp::OnInit() {
    // setup rendering system
    Gfx::Setup(NewGfxDesc().Windowed(400, 400, "Oryol Triangle Sample").Done());
    
    // create a mesh with vertex data from memory
    const float vertices[] = {
        // positions            // colors (RGBA)
         0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f , 1.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f,
    };
    this->drawState.VertexBuffers[0] = Gfx::CreateBuffer(NewBufferDesc()
        .Size(sizeof(vertices))
        .Content(vertices)
        .Done());

    // create shader and pipeline-state-object
    this->drawState.Pipeline = Gfx::CreatePipeline(NewPipelineDesc()
        .Shader(Gfx::CreateShader(Shader::Desc()))
        .Layout(0, {
            { "position", VertexFormat::Float3 },
            { "color0", VertexFormat::Float4 }
        })
        .Done());

    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
TriangleApp::OnRunning() {
    
    Gfx::BeginPass();
    Gfx::ApplyDrawState(this->drawState);
    Gfx::Draw(0, 3);
    Gfx::EndPass();
    Gfx::CommitFrame();
    
    // continue running or quit?
    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

//------------------------------------------------------------------------------
AppState::Code
TriangleApp::OnCleanup() {
    Gfx::Discard();
    return App::OnCleanup();
}
