#pragma once
#include "imgui.h"
#include <algorithm>
#include <vector>
#include <cmath>
#include <glad/gl.h>


struct Colormap {
	const char* name;
	ImVec4(*sample)(float t);
};
extern std::vector<Colormap> colormaps;

ImVec4 Lerp(const ImVec4& a, const ImVec4& b, float t);
ImVec4 SampleGradient(float t, const ImVec4* colors, int count);
ImVec4 SampleViridis(float t);
ImVec4 SamplePlasma(float t);
ImVec4 SampleInferno(float t);
ImVec4 SampleMagma(float t);
ImVec4 SampleCividis(float t);
ImVec4 SampleTurbo(float t);
ImVec4 SampleJet(float t);
ImVec4 SampleCoolWarm(float t);
ImVec4 SampleGrayscale(float t);
ImVec4 SampleRainbow(float t);
ImVec4 SampleOcean(float t);
ImVec4 SampleFire(float t);
void DrawColormapPreview(ImVec4(*sample)(float), ImVec2 size);
bool ColormapCombo(const char* label, int& selected);
void load_colormap(int id, GLuint& colormap);
