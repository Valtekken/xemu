/*
 * Minimal debug/PoC window. NOT trying to match xemu's real ui/xui/
 * look-and-feel or panel-registration pattern (see
 * INTEGRATION_NOTES.md for where a real panel would need to hook
 * in) — this is a self-contained ImGui::Begin/End block, callable
 * from anywhere a frame is already being drawn, whose only job is to
 * make the pipeline's behavior visible and pokeable by hand.
 *
 * TODO(verify): xemu's real ui/xui/ presumably has its own panel
 * registration convention and possibly a themed ImGui context/style
 * already in scope — check main-menu.cc or similar before wiring
 * this in, rather than assuming raw ImGui::Begin() is the right call
 * site.
 */
#include "ra_plugin.h"

#include <imgui.h>
#include <cstdio>

void ra_plugin_draw_debug_ui(bool *p_open)
{
    if (p_open && !*p_open) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(360, 0), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("RetroAchievements (PoC)", p_open)) {
        ImGui::End();
        return;
    }

    ImGui::TextWrapped(
        "No RetroAchievements account or server involved. This arms a "
        "real rcheevos condition and evaluates it against live Xbox "
        "guest RAM every frame, to prove the pipeline itself works.");
    ImGui::Separator();

    static int address = 0;
    static int value = 0;
    static bool is_byte = true;

    ImGui::InputInt("Address (hex offset)", &address, 1, 16, ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::Checkbox("8-bit read", &is_byte);
    ImGui::InputInt("Trigger value", &value);

    if (ImGui::Button("Arm trigger")) {
        ra_plugin_set_poc_trigger((uint32_t)address, is_byte ? 1 : 0, value);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        ra_plugin_poc_reset();
    }

    ImGui::Separator();
    if (ra_plugin_poc_triggered()) {
        ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "UNLOCKED");
    } else {
        ImGui::TextDisabled("Watching...");
    }

    ImGui::End();
}
