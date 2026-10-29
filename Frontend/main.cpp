#include "ChessUI.h"
#include <SDL.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include "PieceAssets.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "third_party/stb_image.h"
#include <cstdio>
#include <fstream>
#include <algorithm>
#include "third_party/json.hpp"
#include <memory>
#include <string>

int main(int argc, char** argv) {
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_GAMECONTROLLER) != 0) { std::fprintf(stderr,"SDL: %s\n",SDL_GetError()); return 1; }
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,"1");
    SDL_Window* window = SDL_CreateWindow("Chess",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,680,900,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) { std::fprintf(stderr,"Window: %s\n",SDL_GetError()); SDL_Quit(); return 1; }
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
    if (!renderer) { std::fprintf(stderr,"Renderer: %s\n",SDL_GetError()); SDL_DestroyWindow(window); SDL_Quit(); return 1; }
    IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGui::GetIO().IniFilename=nullptr;
    float dpi=96;
    SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(window),&dpi,nullptr,nullptr);
    const float scale=std::clamp(dpi/120.0f,1.0f,4.0f);
    ImFontConfig font; font.SizePixels=18.0f*scale; ImGui::GetIO().Fonts->AddFontDefault(&font);
    ImGui::GetStyle().ScaleAllSizes(scale);
    ImGui::GetStyle().TouchExtraPadding={4*scale,4*scale};
    ImGui::StyleColorsDark(); auto& style=ImGui::GetStyle(); style.WindowRounding=0;
    style.Colors[ImGuiCol_WindowBg]={0.09f,0.14f,0.12f,1};
    ImGui_ImplSDL2_InitForSDLRenderer(window,renderer); ImGui_ImplSDLRenderer2_Init(renderer);
    char* preference = SDL_GetPrefPath("Mikaelfk","Chess");
    std::string statePath = preference ? std::string(preference)+"game.json" : ""; SDL_free(preference);
    // --smoke-test avoids touching a user's saved game and exits after real rendering.
    const bool androidSmoke = argc>1 && std::string(argv[1])=="--android-smoke";
    const bool smoke = argc>1 && std::string(argv[1])=="--smoke-test";
    {
        frontend::Game game(smoke?"":statePath); frontend::ChessUI ui(game);
        if (smoke) {
            game.newGame(true,false,0);
            for (auto move : {std::pair{52,36},{11,27},{36,27},{3,27}}) {
                game.beginDrag(move.first); game.drop(move.second);
            }
        }
        SDL_Texture* textures[13][5]{};
        for (int p=1;p<=12;++p) for (int level=0;level<5;++level) {
            int width=0,height=0,channels=0;
            auto* pixels=stbi_load_from_memory(pieceAssets[p][level].data,pieceAssets[p][level].size,&width,&height,&channels,4);
            if (!pixels) { std::fprintf(stderr,"Invalid piece asset %d\n",p); return 2; }
            auto* surface=SDL_CreateRGBSurfaceWithFormatFrom(pixels,width,height,32,width*4,SDL_PIXELFORMAT_RGBA32);
            textures[p][level]=surface?SDL_CreateTextureFromSurface(renderer,surface):nullptr;
            SDL_FreeSurface(surface); stbi_image_free(pixels);
            if (!textures[p][level]) { std::fprintf(stderr,"Piece texture: %s\n",SDL_GetError()); return 2; }
            SDL_SetTextureBlendMode(textures[p][level],SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(textures[p][level],SDL_ScaleModeLinear);
            ui.textures[p][level]=(ImTextureID)(intptr_t)textures[p][level];
        }
        bool running=true; int frames=0; auto previous=SDL_GetPerformanceCounter();
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type==SDL_QUIT) running=false;
                if (event.type==SDL_APP_WILLENTERBACKGROUND) game.setActive(false);
                if (event.type==SDL_APP_DIDENTERFOREGROUND) game.setActive(true);
            }
            const auto now=SDL_GetPerformanceCounter(); game.tick(double(now-previous)/SDL_GetPerformanceFrequency()); previous=now;
            ImGui_ImplSDLRenderer2_NewFrame(); ImGui_ImplSDL2_NewFrame(); ImGui::NewFrame();
            ui.draw(); ImGui::Render();
            if (androidSmoke && frames++ % 30 == 0 && !statePath.empty()) {
                std::ofstream layout(statePath.substr(0,statePath.size()-9)+"ui-layout.json");
                const auto origin=ui.squareCenter(52), destination=ui.squareCenter(36);
                layout << nlohmann::json{{"width",ImGui::GetIO().DisplaySize.x},{"height",ImGui::GetIO().DisplaySize.y},
                    {"from",{origin.x,origin.y}},{"to",{destination.x,destination.y}},
                    {"moves",game.recordedMoves().size()},{"e4",game.position().squares[4][4]}}.dump();
            }
            SDL_SetRenderDrawColor(renderer,23,36,31,255); SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(),renderer);
            if (smoke && ++frames==5) {
                int w,h; SDL_GetRendererOutputSize(renderer,&w,&h);
                auto* screenshot=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_RGBA32);
                if (screenshot && SDL_RenderReadPixels(renderer,nullptr,SDL_PIXELFORMAT_RGBA32,screenshot->pixels,screenshot->pitch)==0) SDL_SaveBMP(screenshot,"chess-smoke.bmp");
                SDL_FreeSurface(screenshot); running=false;
            }
            SDL_RenderPresent(renderer); SDL_Delay(10);
        }
        game.save(); for (const auto& levels:textures) for (auto* texture:levels) if (texture) SDL_DestroyTexture(texture);
    }
    ImGui_ImplSDLRenderer2_Shutdown(); ImGui_ImplSDL2_Shutdown(); ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit(); return 0;
}
