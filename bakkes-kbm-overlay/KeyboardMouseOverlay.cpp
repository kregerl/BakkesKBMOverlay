#include <fstream>
#include "KeyboardMouseOverlay.h"
#include "imgui/imgui.h"

BAKKESMOD_PLUGIN(KeyboardMouseOverlay, "Keyboard Mouse Overlay", "1.0", 0)

void KeyboardMouseOverlay::onLoad() {
	// Num row
	m_inputMap["One"] = { 0, false, "1" };
	m_inputMap["Two"] = { 0, false, "2" };
	m_inputMap["Three"] = { 0, false, "3" };
	m_inputMap["Four"] = { 0, false, "4" };
	m_inputMap["Five"] = { 0, false, "5" };
	// Tab row
	m_inputMap["Tab"] = { 0, false, "Tab" };
	m_inputMap["Q"] = { 0, false, "Q" };
	m_inputMap["W"] = { 0, false, "W" };
	m_inputMap["E"] = { 0, false, "E" };
	m_inputMap["R"] = { 0, false, "R" };
	m_inputMap["T"] = { 0, false, "T" };
	// Caps row
	m_inputMap["CapsLock"] = { 0, false, "Caps" };
	m_inputMap["A"] = { 0, false, "A" };
	m_inputMap["S"] = { 0, false, "S" };
	m_inputMap["D"] = { 0, false, "D" };
	m_inputMap["F"] = { 0, false, "F" };
	m_inputMap["G"] = { 0, false, "G" };
	// Shift row
	m_inputMap["LeftShift"] = { 0, false, "Shift" };
	m_inputMap["Z"] = { 0, false, "Z" };
	m_inputMap["X"] = { 0, false, "X" };
	m_inputMap["C"] = { 0, false, "C" };
	m_inputMap["V"] = { 0, false, "V" };
	// Ctrl row
	m_inputMap["LeftControl"] = { 0, false, "Ctrl" };
	m_inputMap["LeftAlt"] = { 0, false, "Alt" };
	m_inputMap["SpaceBar"] = { 0, false, "Space" };
	// Mouse
	m_inputMap["LeftMouseButton"] = { 0, false, "" };
	m_inputMap["MiddleMouseButton"] = { 0, false, "" };
	m_inputMap["RightMouseButton"] = { 0, false, "" };

	cvarManager->registerCvar(SCALE, "0.75").addOnValueChanged([this](std::string previous, CVarWrapper now) {
		this->m_scale = now.getFloatValue();
		writeCfg();
	});

	cvarManager->registerCvar(TRANSPARENCY, "0").addOnValueChanged([this](std::string previous, CVarWrapper now) {
		this->m_transparency = now.getFloatValue();
		writeCfg();
	});

	for (const auto& input : m_inputMap) {
		cvarManager->registerCvar(input.first, input.first).addOnValueChanged([this](std::string old, CVarWrapper now) {
			m_inputMap[now.getStringValue()].index = gameWrapper->GetFNameIndexByString(now.getStringValue());
			});
		cvarManager->getCvar(input.first).notify();
	}

	if (std::ifstream(m_cfgPath)) {
		cvarManager->loadCfg(m_cfgPath);
	}
	
	gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}, 1);

	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", bind(&KeyboardMouseOverlay::onTick, this, std::placeholders::_1));
}

void KeyboardMouseOverlay::onUnload() {
	writeCfg();
}


void KeyboardMouseOverlay::writeCfg()
{
	std::ofstream stream;
	stream.open(m_cfgPath);

	stream << "kbmscale \"" + std::to_string(m_scale) + "\"";
	stream << "\n";
	stream << "kbmtransparency \"" + std::to_string(m_transparency) + "\"";
	stream << "\n";

	stream.close();
}

void KeyboardMouseOverlay::onTick(std::string eventName) 
{
	if (!gameWrapper->IsInCustomTraining()) {
		if (gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) {
			for (auto const& entry : m_inputMap) {
				if (entry.second.index > 0) {
					m_inputMap[entry.first].pressed = gameWrapper->IsKeyPressed(entry.second.index);
				}
			}
		}
	}
}

void KeyboardMouseOverlay::Render()
{
	if (!m_renderOverlay) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());
		return;
	}

	if (!gameWrapper->IsInCustomTraining()) {
		ServerWrapper server = NULL;
		if (gameWrapper->IsInOnlineGame() ) {
			server = gameWrapper->GetOnlineGame();
		}
		else if (gameWrapper->IsInGame()) {
			server = gameWrapper->GetGameEventAsServer();
		}
		
		if (!server.IsNull() && !server.GetbMatchEnded()) {
			ImGui::SetNextWindowBgAlpha(m_transparency);
			ImGui::SetNextWindowPos(ImVec2(128, 128), ImGuiCond_FirstUseEver);

			ImVec2 windowSize = ImVec2(m_scale * WINDOW_WIDTH, m_scale * WINDOW_HEIGHT);
			ImGui::SetNextWindowSize(windowSize);

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

			ImGui::Begin(GetMenuTitle().c_str(), &m_renderOverlay, windowFlags);

			ImDrawList* drawlist = ImGui::GetWindowDrawList();
			ImVec2 screenPos = ImGui::GetCursorScreenPos();

			// Margin
			screenPos.x += 12;
			screenPos.y += 24;

			// Draw partial keyboard overlay
			keyPressHelper(drawlist, { "One", "Two", "Three", "Four", "Five" }, ImVec2(screenPos.x + 75, screenPos.y));
			renderTabRow(drawlist, screenPos);
			renderCapsRow(drawlist, screenPos);
			renderShiftRow(drawlist, screenPos);
			renderCtrlRow(drawlist, screenPos);

			// Draw mouse
			int mouseXPos = screenPos.x + (500 * m_scale);
			int mouseMarginTop = 45;

			int lmbWidth = (80 * m_scale);
			int lmbHeight = (200 * m_scale);
			drawlist->AddRectFilled(ImVec2(mouseXPos, screenPos.y + mouseMarginTop), ImVec2(mouseXPos + lmbWidth, screenPos.y + mouseMarginTop + lmbHeight), getColor(m_inputMap.at("LeftMouseButton").pressed), 40, ImDrawCornerFlags_TopLeft);
			drawlist->AddRectFilled(ImVec2(mouseXPos + lmbWidth, screenPos.y + mouseMarginTop), ImVec2(mouseXPos + lmbWidth + (20 * m_scale), screenPos.y + mouseMarginTop + (lmbHeight * 0.5f)), getColor(m_inputMap.at("MiddleMouseButton").pressed), 0, ImDrawCornerFlags_None);
			drawlist->AddRectFilled(ImVec2(mouseXPos + lmbWidth, screenPos.y + mouseMarginTop + (lmbHeight * 0.5f)), ImVec2(mouseXPos + lmbWidth + (20 * m_scale), screenPos.y + mouseMarginTop + lmbHeight), COLOR_BLACK, 0, ImDrawCornerFlags_None);
			drawlist->AddRectFilled(ImVec2(mouseXPos + lmbWidth, screenPos.y + mouseMarginTop + (lmbHeight * 0.5f)), ImVec2(mouseXPos + lmbWidth + (20 * m_scale), screenPos.y + mouseMarginTop + (lmbHeight * 0.5f)), COLOR_BLACK, 0, ImDrawCornerFlags_None);
			drawlist->AddRectFilled(ImVec2(mouseXPos + lmbWidth + (20 * m_scale), screenPos.y + mouseMarginTop), ImVec2(mouseXPos + (lmbWidth * 2) + (20 * m_scale), screenPos.y + mouseMarginTop + lmbHeight), getColor(m_inputMap.at("RightMouseButton").pressed), 40, ImDrawCornerFlags_TopRight);

			ImGui::PopStyleVar();
			ImGui::End();
		}
	}
}



std::string KeyboardMouseOverlay::GetMenuName() {
	return "kbmoverlay";
}

std::string KeyboardMouseOverlay::GetMenuTitle() {
	return "KBM Overlay";
}

void KeyboardMouseOverlay::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

bool KeyboardMouseOverlay::ShouldBlockInput() {
	return false;
}

bool KeyboardMouseOverlay::IsActiveOverlay() {
	return false;
}

void KeyboardMouseOverlay::OnOpen() {
	m_renderOverlay = true;
}

void KeyboardMouseOverlay::OnClose() {
	m_renderOverlay = false;
}

void KeyboardMouseOverlay::renderTabRow(ImDrawList* drawlist, ImVec2 screenPos) {
	int tabYPos = screenPos.y + scaledButtonSize() + BUTTON_MARGIN;
	int width = (scaledButtonSize() * 2);
	ImVec2 textSize = ImGui::CalcTextSize("Tab");
	drawlist->AddRectFilled(ImVec2(screenPos.x, tabYPos), ImVec2(screenPos.x + width, tabYPos + scaledButtonSize()), getColor(m_inputMap.at("Tab").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(screenPos.x + (width * 0.5f) - (textSize.x * 0.5f), tabYPos + (scaledButtonSize() * 0.5f) - (textSize.y * 0.5f)), COLOR_WHITE, "Tab");

	keyPressHelper(drawlist, { "Q", "W", "E", "R", "T" }, ImVec2(screenPos.x + (scaledButtonSize() * 2) + BUTTON_MARGIN, tabYPos));
}

void KeyboardMouseOverlay::renderCapsRow(ImDrawList* drawlist, ImVec2 screenPos) {
	int capsYPos = screenPos.y + ((scaledButtonSize() + BUTTON_MARGIN) * 2);
	int capsWidth = ((BUTTON_SIZE * 2) + 16) * m_scale;
	ImVec2 textSize = ImGui::CalcTextSize("Caps");
	drawlist->AddRectFilled(ImVec2(screenPos.x, capsYPos), ImVec2(screenPos.x + capsWidth, capsYPos + scaledButtonSize()), getColor(m_inputMap.at("CapsLock").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(screenPos.x + (capsWidth * 0.5f) - (textSize.x * 0.5f), capsYPos + (scaledButtonSize() * 0.5f) - (textSize.y * 0.5f)), COLOR_WHITE, "Caps");

	keyPressHelper(drawlist, { "A", "S", "D", "F", "G" }, ImVec2(screenPos.x + capsWidth + BUTTON_MARGIN, capsYPos));
}

void KeyboardMouseOverlay::renderShiftRow(ImDrawList* drawlist, ImVec2 screenPos) {
	int shiftYPos = screenPos.y + ((scaledButtonSize() + BUTTON_MARGIN) * 3);
	int shiftWidth = ((BUTTON_SIZE * 2) +16 + 16) * m_scale;
	ImVec2 textSize = ImGui::CalcTextSize("Shift");
	drawlist->AddRectFilled(ImVec2(screenPos.x, shiftYPos), ImVec2(screenPos.x + shiftWidth, shiftYPos + scaledButtonSize()), getColor(m_inputMap.at("LeftShift").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(screenPos.x + (shiftWidth * 0.5f) - (textSize.x * 0.5f), shiftYPos + (scaledButtonSize() * 0.5f) - (textSize.x * 0.5f)), COLOR_WHITE, "Shift");

	keyPressHelper(drawlist, { "Z", "X", "C", "V" }, ImVec2(screenPos.x + shiftWidth + BUTTON_MARGIN, shiftYPos));
}

void KeyboardMouseOverlay::renderCtrlRow(ImDrawList* drawlist, ImVec2 screenPos) {
	int ctrlYPos = screenPos.y + ((scaledButtonSize() + BUTTON_MARGIN) * 4);
	int ctrlWidth = (BUTTON_SIZE + 16 + 16) * m_scale;
	ImVec2 ctrlTextSize = ImGui::CalcTextSize("Ctrl");
	drawlist->AddRectFilled(ImVec2(screenPos.x, ctrlYPos), ImVec2(screenPos.x + ctrlWidth, ctrlYPos + scaledButtonSize()), getColor(m_inputMap.at("LeftControl").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(screenPos.x + (ctrlWidth * 0.5f) - (ctrlTextSize.x * 0.5f), ctrlYPos + (scaledButtonSize() * 0.5f) - (ctrlTextSize.y * 0.5f)), COLOR_WHITE, "Ctrl");

	int altXPos = screenPos.x + ctrlWidth + scaledButtonSize() + BUTTON_MARGIN;
	ImVec2 altTextSize = ImGui::CalcTextSize("Alt");
	drawlist->AddRectFilled(ImVec2(altXPos, ctrlYPos), ImVec2(altXPos + ctrlWidth, ctrlYPos + scaledButtonSize()), getColor(m_inputMap.at("LeftAlt").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(altXPos + (ctrlWidth / 2) - (altTextSize.x * 0.5f), ctrlYPos + (scaledButtonSize() * 0.5f) - (altTextSize.y * 0.5f)), COLOR_WHITE, "Alt");


	int spaceXPos = altXPos + ctrlWidth + BUTTON_MARGIN;
	int spaceWidth = (scaledButtonSize() * 4) + BUTTON_MARGIN;
	ImVec2 spaceTextSize = ImGui::CalcTextSize("Space");
	drawlist->AddRectFilled(ImVec2(spaceXPos, ctrlYPos), ImVec2(spaceXPos + spaceWidth, ctrlYPos + scaledButtonSize()), getColor(m_inputMap.at("SpaceBar").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(spaceXPos + (spaceWidth / 2) - (spaceTextSize.x * 0.5f), ctrlYPos + (scaledButtonSize() * 0.5f) - (spaceTextSize.y * 0.5f)), COLOR_WHITE, "Space");
}

void KeyboardMouseOverlay::keyPressHelper(ImDrawList* drawlist, const std::vector<std::string> &keys, ImVec2 startingPos) {
	for (int i = 0; i < keys.size(); i++) {
		const std::string& key = keys[i];
		ImVec2 textSize = ImGui::CalcTextSize(m_inputMap.at(key).name.c_str());
		ImVec2 position = ImVec2(startingPos.x + (i * scaledButtonSize()) + (BUTTON_MARGIN * i), startingPos.y);

		drawlist->AddRectFilled(position, ImVec2(position.x + scaledButtonSize(), position.y + scaledButtonSize()), getColor(m_inputMap.at(key).pressed), 0, ImDrawCornerFlags_None);
		drawlist->AddText(ImVec2(position.x + (scaledButtonSize() * 0.5f) - (textSize.x * 0.5f), position.y + (scaledButtonSize() * 0.5f) - (textSize.y * 0.5f)), COLOR_WHITE, m_inputMap.at(key).name.c_str());
	}
}

ImColor KeyboardMouseOverlay::getColor(bool isEnabled) {
	return isEnabled ? COLOR_GRAY : COLOR_BLACK;
}