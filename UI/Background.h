#pragma once

#include <cstdint>
#include "Common/File/Path.h"

class UIContext;

// Variable global del boot ISO
extern Path boot_filename;

// Funciones del background
void UIBackgroundInit(UIContext &dc);
void UIBackgroundShutdown();
void DrawGameBackground(UIContext &dc, const Path &gamePath, float x, float y, float z);
void DrawBackground(UIContext &dc, float alpha, float x, float y, float z);
uint32_t GetBackgroundColorWithAlpha(const UIContext &dc);
