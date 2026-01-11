#include <cmath>
#include <vector>
#include "UI/UI.h"                 // para ui_draw2d
#include "Common/Data/Random/Rng.h" // para GMRng
#include "Common/Math/geom2d.h"
#include "UI/Background.h"
#include "Common/GPU/thin3d.h"
#include "Common/UI/Context.h"
#include "Common/Data/Color/RGBAUtil.h"
#include "Common/File/Path.h"
#include "Common/TimeUtil.h"
#include "Common/Render/ManagedTexture.h"
#include "Core/Config.h"

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

static Draw::Texture *bgTexture = nullptr;

 

// ------------------ ANIMATION BASE ------------------
class Animation {
public:
    virtual ~Animation() = default;
    virtual void Draw(UIContext &dc, double t, float alpha, float x, float y, float z) = 0;
};

// ------------------ FIAMY FLOATING ANIMATION ------------------
class FiamyFloatingAnimation : public Animation {
public:
    FiamyFloatingAnimation(bool is_colored = true) : is_colored(is_colored) {}

    void Draw(UIContext &dc, double t, float alpha, float x, float y, float z) override {
        float xres = dc.GetBounds().w;
        float yres = dc.GetBounds().h;

        dc.Flush();
        dc.Begin();
        if (last_xres != xres || last_yres != yres) {
            Regenerate(xres, yres);
        }

        for (size_t i = 0; i < base.size(); i++) {
            const float px = base[i].x + dc.GetBounds().x;
            const float py = base[i].y + dc.GetBounds().y + 20 * cosf(i * 5.0f + t * 0.1f); // Más lento
            const float angle = sinf(i + t * 0.1f); // Más lento
            uint32_t color = is_colored ? colorAlpha(0xFFFFC0CB, alpha * 0.3f) : colorAlpha(0xFFDDC0DD, alpha * 0.2f);
            ui_draw2d.DrawImageRotated(ImageID("I_FIAMY"), px, py, 0.5f, angle, color); // Más pequeño
        }

        dc.Flush();
    }

private:
    bool is_colored = true;
    std::vector<Point2D> base;
    float last_xres = 0;
    float last_yres = 0;

    void Regenerate(int xres, int yres) {
        int count = static_cast<int>(xres * yres / (300.0f * 300.0f));  // Menos cantidad
        base.resize(count);

        GMRng rng;
        rng.Init(time_now_d() * 100239);
        for (size_t i = 0; i < base.size(); i++) {
            // ✅ Asignación correcta de Point2D
            base[i] = Point2D(rng.F() * xres, rng.F() * yres);
        }

        last_xres = xres;
        last_yres = yres;
    }
};

// ------------------ VARIABLES GLOBALES ------------------
static BackgroundAnimation g_CurBackgroundAnimation = BackgroundAnimation::OFF;
static std::unique_ptr<Animation> g_Animation;
static bool bgTextureInited = false;

void UIBackgroundInit(UIContext &dc) {
    bgTexture = nullptr; // fondo degradé rosa pastel

    // Ya no se hace LoadImage. El ImageID se usa directamente al dibujar.
    // ImageID id("I_FIAMY");
    // Path fiamyPath("assets/ui_images/fiamy.png");
    // ui_draw2d.LoadImage(id, fiamyPath);
}

// Devuelve color de fondo rosa pastel con alpha
uint32_t GetBackgroundColorWithAlpha(const UIContext &dc) {
    return colorAlpha(0xFFFFC0CB, 0.65f);
}

// ------------------ DIBUJAR FONDO ------------------
void DrawBackground(UIContext &dc, float alpha, float x, float y, float z) {
    if (!bgTextureInited) {
        UIBackgroundInit(dc);
        bgTextureInited = true;
    }

    if (g_CurBackgroundAnimation != static_cast<BackgroundAnimation>(g_Config.iBackgroundAnimation)) {
        g_CurBackgroundAnimation = static_cast<BackgroundAnimation>(g_Config.iBackgroundAnimation);

        switch (g_CurBackgroundAnimation) {
            case BackgroundAnimation::FLOATING_SYMBOLS:
            case BackgroundAnimation::FLOATING_SYMBOLS_COLORED:
                g_Animation.reset(new FiamyFloatingAnimation(true));
                break;
            default:
                g_Animation.reset(nullptr);
        }
    }

    // Fondo rosa pastel
    Bounds bounds = dc.GetBounds();
    dc.Flush();
    dc.BeginNoTex();
    uint32_t topColor = colorAlpha(0xFFFFC0CB, alpha);
    uint32_t bottomColor = colorAlpha(0xFFFFE0F0, alpha);
    dc.Draw()->RectVGradient(bounds.x, bounds.y, bounds.x + bounds.w, bounds.y + bounds.h, topColor, bottomColor);
    dc.Flush();

    double t = time_now_d();
    if (g_Animation) {
        g_Animation->Draw(dc, t, alpha, x, y, z);
    }
}

// ------------------ DIBUJAR FONDO DE JUEGO ------------------
void DrawGameBackground(UIContext &dc, const Path &gamePath, float x, float y, float z) {
    if (gamePath.empty()) {
        DrawBackground(dc, 1.0f, x, y, z);
        return;
    }

    // Por ahora, usa DrawBackground normal
    DrawBackground(dc, 1.0f, x, y, z);
}

// ------------------ CIERRE ------------------
void UIBackgroundShutdown() {
    if (bgTexture) {
        bgTexture->Release();
        bgTexture = nullptr;
    }
    bgTextureInited = false;
    g_Animation.reset(nullptr);
    g_CurBackgroundAnimation = BackgroundAnimation::OFF;
}
