#pragma once
#pragma comment(lib, "pluginsdk.lib")

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "imgui/imgui.h"

#define WINDOW_WIDTH 768
#define WINDOW_HEIGHT 384

#define BUTTON_SIZE 50
#define BUTTON_MARGIN 5

#define COLOR_WHITE ImColor(255, 255, 255)
#define COLOR_BLACK ImColor(0, 0, 0)
#define COLOR_GRAY ImColor(156, 156, 156)

#define SCALE "kbmscale"
#define TRANSPARENCY "kbmtransparency"

class KeyboardMouseOverlay : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow {

public:
	void onLoad() override;

	void onUnload() override;

	void onTick(std::string eventName);

	void Render() override;

	virtual std::string GetMenuName() override;

	virtual std::string GetMenuTitle() override;

	virtual void SetImGuiContext(uintptr_t ctx) override;

	virtual bool ShouldBlockInput() override;

	virtual bool IsActiveOverlay() override;

	virtual void OnOpen() override;

	virtual void OnClose() override;

private:
	void renderTabRow(ImDrawList* drawlist, ImVec2 screenPos);
	
	void renderCapsRow(ImDrawList* drawlist, ImVec2 screenPos);

	void renderShiftRow(ImDrawList* drawlist, ImVec2 screenPos);

	void renderCtrlRow(ImDrawList* drawlist, ImVec2 screenPos);

	void keyPressHelper(ImDrawList* drawlist, const std::vector<std::string> &keys, ImVec2 startingPos);

	ImColor getColor(bool isEnabled);

	void writeCfg();

	int scaledButtonSize() { return BUTTON_SIZE * m_scale; }

private:
	struct Input {
		int index;
		bool pressed;
		std::string name;
	};
	
	bool m_renderOverlay = false;
	float m_scale = 1.0f;
	float m_transparency = 1.0f;
	std::string m_cfgPath = "./bakkesmod/cfg/kbmoverlay.cfg";
	std::map<std::string, Input> m_inputMap;
};
