#include "imgui.h"
#include <algorithm>
#include <cmath>


struct Colormap {
	const char* name;
	ImVec4(*sample)(float t);
};

static ImVec4 Lerp(const ImVec4& a, const ImVec4& b, float t) {
	return ImVec4(
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t,
		a.z + (b.z - a.z) * t,
		a.w + (b.w - a.w) * t
	);
}

static ImVec4 SampleGradient(float t, const ImVec4* colors, int count) {
	t = std::clamp(t, 0.0f, 1.0f);
	float x = t * (count - 1);
	int i = std::min((int)x, count - 2);
	return Lerp(colors[i], colors[i + 1], x - i);
}

static ImVec4 SampleViridis(float t) {
	static const ImVec4 colors[] = {
		{ 0.267f, 0.005f, 0.329f, 1.0f },
		{ 0.283f, 0.141f, 0.458f, 1.0f },
		{ 0.254f, 0.265f, 0.530f, 1.0f },
		{ 0.164f, 0.471f, 0.558f, 1.0f },
		{ 0.135f, 0.659f, 0.518f, 1.0f },
		{ 0.478f, 0.821f, 0.318f, 1.0f },
		{ 0.993f, 0.906f, 0.144f, 1.0f }
	};

	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}

static ImVec4 SamplePlasma(float t) {
	static const ImVec4 colors[] = {
		{ 0.050f, 0.030f, 0.528f, 1.0f },
		{ 0.287f, 0.010f, 0.627f, 1.0f },
		{ 0.500f, 0.050f, 0.650f, 1.0f },
		{ 0.710f, 0.180f, 0.600f, 1.0f },
		{ 0.880f, 0.390f, 0.440f, 1.0f },
		{ 0.980f, 0.650f, 0.220f, 1.0f },
		{ 0.940f, 0.975f, 0.130f, 1.0f }
	};
	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}
static ImVec4 SampleInferno(float t) {
	static const ImVec4 colors[] = {
		{ 0.001f, 0.000f, 0.014f, 1.0f },
		{ 0.090f, 0.016f, 0.150f, 1.0f },
		{ 0.259f, 0.039f, 0.407f, 1.0f },
		{ 0.578f, 0.148f, 0.404f, 1.0f },
		{ 0.865f, 0.317f, 0.227f, 1.0f },
		{ 0.988f, 0.652f, 0.125f, 1.0f },
		{ 0.988f, 0.998f, 0.645f, 1.0f }
	};
	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}

static ImVec4 SampleMagma(float t) {
	static const ImVec4 colors[] = {
		{ 0.001f, 0.000f, 0.014f, 1.0f },
		{ 0.090f, 0.030f, 0.250f, 1.0f },
		{ 0.300f, 0.070f, 0.500f, 1.0f },
		{ 0.550f, 0.130f, 0.500f, 1.0f },
		{ 0.800f, 0.250f, 0.450f, 1.0f },
		{ 0.950f, 0.500f, 0.350f, 1.0f },
		{ 0.990f, 0.950f, 0.700f, 1.0f }
	};

	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}

static ImVec4 SampleCividis(float t) {
	static const ImVec4 colors[] = {
		{ 0.000f, 0.135f, 0.305f, 1.0f },
		{ 0.120f, 0.250f, 0.420f, 1.0f },
		{ 0.250f, 0.360f, 0.470f, 1.0f },
		{ 0.400f, 0.470f, 0.480f, 1.0f },
		{ 0.570f, 0.570f, 0.440f, 1.0f },
		{ 0.750f, 0.680f, 0.350f, 1.0f },
		{ 0.996f, 0.910f, 0.145f, 1.0f }
	};

	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}

static ImVec4 SampleTurbo(float t) {
	static const ImVec4 colors[] = {
		{ 0.190f, 0.071f, 0.232f, 1.0f },
		{ 0.086f, 0.408f, 0.820f, 1.0f },
		{ 0.012f, 0.690f, 0.650f, 1.0f },
		{ 0.380f, 0.870f, 0.380f, 1.0f },
		{ 0.900f, 0.850f, 0.120f, 1.0f },
		{ 0.980f, 0.450f, 0.100f, 1.0f },
		{ 0.480f, 0.015f, 0.010f, 1.0f }
	};

	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}

static ImVec4 SampleJet(float t) {
	static const ImVec4 colors[] = {
		{ 0.000f, 0.000f, 0.500f, 1.0f },
		{ 0.000f, 0.500f, 1.000f, 1.0f },
		{ 0.000f, 1.000f, 1.000f, 1.0f },
		{ 0.500f, 1.000f, 0.500f, 1.0f },
		{ 1.000f, 1.000f, 0.000f, 1.0f },
		{ 1.000f, 0.500f, 0.000f, 1.0f },
		{ 0.500f, 0.000f, 0.000f, 1.0f }
	};

	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}

static ImVec4 SampleCoolWarm(float t) {
	static const ImVec4 colors[] = {
		{ 0.230f, 0.299f, 0.754f, 1.0f },
		{ 0.440f, 0.580f, 0.950f, 1.0f },
		{ 0.670f, 0.780f, 0.990f, 1.0f },
		{ 0.865f, 0.865f, 0.865f, 1.0f },
		{ 0.960f, 0.680f, 0.560f, 1.0f },
		{ 0.870f, 0.350f, 0.280f, 1.0f },
		{ 0.706f, 0.016f, 0.150f, 1.0f }
	};

	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}

static ImVec4 SampleGrayscale(float t)
{
	static const ImVec4 colors[] =
	{
		{ 0.000f, 0.000f, 0.000f, 1.0f },
		{ 0.250f, 0.250f, 0.250f, 1.0f },
		{ 0.500f, 0.500f, 0.500f, 1.0f },
		{ 0.750f, 0.750f, 0.750f, 1.0f },
		{ 1.000f, 1.000f, 1.000f, 1.0f }
	};

	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}

static ImVec4 SampleRainbow(float t) {
	static const ImVec4 colors[] = {
		{ 0.500f, 0.000f, 1.000f, 1.0f },
		{ 0.000f, 0.000f, 1.000f, 1.0f },
		{ 0.000f, 1.000f, 1.000f, 1.0f },
		{ 0.000f, 1.000f, 0.000f, 1.0f },
		{ 1.000f, 1.000f, 0.000f, 1.0f },
		{ 1.000f, 0.500f, 0.000f, 1.0f },
		{ 1.000f, 0.000f, 0.000f, 1.0f }
	};

	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}

static ImVec4 SampleOcean(float t) {
	static const ImVec4 colors[] = {
		{ 0.000f, 0.050f, 0.150f, 1.0f },
		{ 0.000f, 0.180f, 0.350f, 1.0f },
		{ 0.000f, 0.400f, 0.550f, 1.0f },
		{ 0.000f, 0.650f, 0.700f, 1.0f },
		{ 0.200f, 0.850f, 0.800f, 1.0f },
		{ 0.600f, 0.950f, 0.850f, 1.0f },
		{ 0.900f, 1.000f, 0.950f, 1.0f }
	};

	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}

static ImVec4 SampleFire(float t) {
	static const ImVec4 colors[] = {
		{ 0.000f, 0.000f, 0.000f, 1.0f },
		{ 0.250f, 0.000f, 0.000f, 1.0f },
		{ 0.600f, 0.020f, 0.000f, 1.0f },
		{ 0.900f, 0.150f, 0.000f, 1.0f },
		{ 1.000f, 0.400f, 0.000f, 1.0f },
		{ 1.000f, 0.750f, 0.050f, 1.0f },
		{ 1.000f, 1.000f, 0.850f, 1.0f }
	};

	return SampleGradient(t, colors, IM_ARRAYSIZE(colors));
}


static void DrawColormapPreview(ImVec4(*sample)(float), ImVec2 size) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 p = ImGui::GetCursorScreenPos();

	const int segments = 64;
	for (int i = 0; i < segments; ++i) {
		float t0 = (float)i / segments;
		float t1 = (float)(i + 1) / segments;

		ImU32 color = ImGui::ColorConvertFloat4ToU32(
			sample((t0 + t1) * 0.5f)
		);

		draw_list->AddRectFilled(
			ImVec2(p.x + size.x * t0, p.y),
			ImVec2(p.x + size.x * t1 + 1.0f, p.y + size.y),
			color
		);
	}

	// Border
	draw_list->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), ImGui::GetColorU32(ImGuiCol_Border));

	// Make ImGui advance its cursor.
	ImGui::Dummy(size);
}
static const Colormap colormaps[] = {
	{ "Viridis",   SampleViridis   },
	{ "Plasma",    SamplePlasma    },
	{ "Inferno",   SampleInferno   },
	{ "Magma",     SampleMagma     },
	{ "Cividis",   SampleCividis   },
	{ "Turbo",     SampleTurbo     },
	{ "Jet",       SampleJet       },
	{ "CoolWarm",  SampleCoolWarm  },
	{ "Grayscale", SampleGrayscale },
	{ "Rainbow",   SampleRainbow   },
	{ "Ocean",     SampleOcean     },
	{ "Fire",      SampleFire      },
};
static bool ColormapCombo(const char* label,int& selected){


	const int count = IM_ARRAYSIZE(colormaps);
	bool changed = false;

	if (ImGui::BeginCombo("Colormap", colormaps[selected].name)) {
		const float preview_width = 140.0f;
		const float preview_height = 10.0f;
		const float right_padding = 8.0f;

		for (int i = 0; i < IM_ARRAYSIZE(colormaps); ++i) {
			bool is_selected = (selected == i);

			ImGui::PushID(i);

			ImVec2 item_pos = ImGui::GetCursorScreenPos();
			float item_width = ImGui::GetContentRegionAvail().x;

			// Toute la ligne est cliquable
			if (ImGui::Selectable("##colormap", is_selected, ImGuiSelectableFlags_None, ImVec2(item_width, 10.0f)))
				selected = i;

			// Texte
			ImGui::SetCursorScreenPos(ImVec2(item_pos.x + 4.0f, item_pos.y - 2.0f));
			ImGui::TextUnformatted(colormaps[i].name);
			// Preview alignée à droite
			float preview_x = item_pos.x + item_width - preview_width - right_padding;
			ImGui::SetCursorScreenPos(ImVec2(preview_x, item_pos.y + 0.0f));

			DrawColormapPreview(colormaps[i].sample, ImVec2(preview_width, preview_height));
			if (is_selected) ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	return changed;
}


static void load_colormap(int id, GLuint& colormap) {
	glGenTextures(1, &colormap);
	glBindTexture(GL_TEXTURE_1D, colormap);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	unsigned char data[3 * 64];
	FOR(p, 64)FOR(c, 3) {
		auto sample = colormaps[id].sample(float(p)/63.);
		data[3 * p + 0] = sample.x * 255.;
		data[3 * p + 1] = sample.y * 255.;
		data[3 * p + 2] = sample.z * 255.;
	}
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, data); 
}
