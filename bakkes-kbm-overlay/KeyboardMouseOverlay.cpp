
#include "KeyboardMouseOverlay.h"
#include "imgui/imgui.h"

BAKKESMOD_PLUGIN(KeyboardMouseOverlay, "Keyboard Mouse Overlay", "1.0", 0)

void KeyboardMouseOverlay::onLoad() {
	// Num row
	inputMap["One"] = { 0, false, "1" };
	inputMap["Two"] = { 0, false, "2" };
	inputMap["Three"] = { 0, false, "3" };
	inputMap["Four"] = { 0, false, "4" };
	inputMap["Five"] = { 0, false, "5" };
	// Tab row
	inputMap["Tab"] = { 0, false, "Tab" };
	inputMap["Q"] = { 0, false, "Q" };
	inputMap["W"] = { 0, false, "W" };
	inputMap["E"] = { 0, false, "E" };
	inputMap["R"] = { 0, false, "R" };
	inputMap["T"] = { 0, false, "T" };
	// Caps row
	inputMap["CapsLock"] = { 0, false, "Caps" };
	inputMap["A"] = { 0, false, "A" };
	inputMap["S"] = { 0, false, "S" };
	inputMap["D"] = { 0, false, "D" };
	inputMap["F"] = { 0, false, "F" };
	inputMap["G"] = { 0, false, "G" };
	// Shift row
	inputMap["LeftShift"] = { 0, false, "Shift" };
	inputMap["Z"] = { 0, false, "Z" };
	inputMap["X"] = { 0, false, "X" };
	inputMap["C"] = { 0, false, "C" };
	inputMap["V"] = { 0, false, "V" };
	// Ctrl row
	inputMap["LeftControl"] = { 0, false, "Ctrl" };
	inputMap["LeftAlt"] = { 0, false, "Alt" };
	inputMap["SpaceBar"] = { 0, false, "Space" };
	// Mouse
	inputMap["LeftMouseButton"] = { 0, false, "" };
	inputMap["MiddleMouseButton"] = { 0, false, "" };
	inputMap["RightMouseButton"] = { 0, false, "" };

	for (const auto& input : inputMap) {
		cvarManager->registerCvar(input.first, input.first).addOnValueChanged([this](std::string old, CVarWrapper now) {
			inputMap[now.getStringValue()].index = gameWrapper->GetFNameIndexByString(now.getStringValue());
			});
		cvarManager->getCvar(input.first).notify();
	}

	gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());
		}, 1);

	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", bind(&KeyboardMouseOverlay::onTick, this, std::placeholders::_1));
}

void KeyboardMouseOverlay::onUnload() {


}

void KeyboardMouseOverlay::onTick(std::string eventName) 
{
	if (!gameWrapper->IsInCustomTraining()) {
		if (gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) {
			for (auto const& entry : inputMap) {
				if (entry.second.index > 0) {
					inputMap[entry.first].pressed = gameWrapper->IsKeyPressed(entry.second.index);
				}
			}
		}
	}
}

void KeyboardMouseOverlay::Render()
{
	if (!renderOverlay) {
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
			ImGui::SetNextWindowBgAlpha(.25F);
			ImGui::SetNextWindowPos(ImVec2(128, 128), ImGuiCond_FirstUseEver);

			ImVec2 windowSize = ImVec2(768, 384);
			ImGui::SetNextWindowSize(windowSize);

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

			ImGui::Begin(GetMenuTitle().c_str(), &renderOverlay, windowFlags);

			ImDrawList* drawlist = ImGui::GetWindowDrawList();
			ImVec2 screenPos = ImGui::GetCursorScreenPos();

			// Margin
			screenPos.x += 12;

			// Draw partial keyboard overlay
			keyPressHelper(drawlist, { "One", "Two", "Three", "Four", "Five" }, ImVec2(screenPos.x + 75, screenPos.y));
			renderTabRow(drawlist, screenPos);
			renderCapsRow(drawlist, screenPos);
			renderShiftRow(drawlist, screenPos);
			renderCtrlRow(drawlist, screenPos);

			// Draw mouse
			int mouseXPos = screenPos.x + 500;
			int mouseMarginTop = 45;
			drawlist->AddRectFilled(ImVec2(mouseXPos, screenPos.y + mouseMarginTop), ImVec2(mouseXPos + 80, screenPos.y + mouseMarginTop + 200), getColor(inputMap.at("LeftMouseButton").pressed), 40, ImDrawCornerFlags_TopLeft);
			drawlist->AddRectFilled(ImVec2(mouseXPos + 80, screenPos.y + mouseMarginTop), ImVec2(mouseXPos + 80 + 20, screenPos.y + mouseMarginTop + 100), getColor(inputMap.at("MiddleMouseButton").pressed), 0, ImDrawCornerFlags_None);
			drawlist->AddRectFilled(ImVec2(mouseXPos + 80, screenPos.y + mouseMarginTop + 100), ImVec2(mouseXPos + 80 + 20, screenPos.y + mouseMarginTop + 200), COLOR_BLACK, 0, ImDrawCornerFlags_None);
			drawlist->AddRectFilled(ImVec2(mouseXPos + 100, screenPos.y + mouseMarginTop), ImVec2(mouseXPos + 100 + 80, screenPos.y + mouseMarginTop + 200), getColor(inputMap.at("RightMouseButton").pressed), 40, ImDrawCornerFlags_TopRight);

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
	renderOverlay = true;
}

void KeyboardMouseOverlay::OnClose() {
	renderOverlay = false;
}

void KeyboardMouseOverlay::renderTabRow(ImDrawList* drawlist, ImVec2 screenPos) {
	int tabYPos = screenPos.y + BUTTON_SIZE + BUTTON_MARGIN;
	drawlist->AddRectFilled(ImVec2(screenPos.x, tabYPos), ImVec2(screenPos.x + (BUTTON_SIZE * 2), tabYPos + BUTTON_SIZE), getColor(inputMap.at("Tab").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(screenPos.x + BUTTON_MARGIN, tabYPos + (BUTTON_SIZE / 2) - BUTTON_MARGIN), COLOR_WHITE, "Tab");

	keyPressHelper(drawlist, { "Q", "W", "E", "R", "T" }, ImVec2(screenPos.x + (BUTTON_SIZE * 2) + BUTTON_MARGIN, tabYPos));
}

void KeyboardMouseOverlay::renderCapsRow(ImDrawList* drawlist, ImVec2 screenPos) {
	int capsYPos = screenPos.y + ((BUTTON_SIZE + BUTTON_MARGIN) * 2);
	int capsWidth = (BUTTON_SIZE * 2) + 16;
	drawlist->AddRectFilled(ImVec2(screenPos.x, capsYPos), ImVec2(screenPos.x + capsWidth, capsYPos + BUTTON_SIZE), getColor(inputMap.at("CapsLock").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(screenPos.x + BUTTON_MARGIN, capsYPos + (BUTTON_SIZE / 2) - BUTTON_MARGIN), COLOR_WHITE, "Caps");

	keyPressHelper(drawlist, { "A", "S", "D", "F", "G" }, ImVec2(screenPos.x + capsWidth + BUTTON_MARGIN, capsYPos));
}

void KeyboardMouseOverlay::renderShiftRow(ImDrawList* drawlist, ImVec2 screenPos) {
	int shiftYPos = screenPos.y + ((BUTTON_SIZE + BUTTON_MARGIN) * 3);
	int shiftWidth = (BUTTON_SIZE * 2) + 16 + 16;
	drawlist->AddRectFilled(ImVec2(screenPos.x, shiftYPos), ImVec2(screenPos.x + shiftWidth, shiftYPos + BUTTON_SIZE), getColor(inputMap.at("LeftShift").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(screenPos.x + BUTTON_MARGIN, shiftYPos + (BUTTON_SIZE / 2) - BUTTON_MARGIN), COLOR_WHITE, "Shift");

	keyPressHelper(drawlist, { "Z", "X", "C", "V" }, ImVec2(screenPos.x + shiftWidth + BUTTON_MARGIN, shiftYPos));
}

void KeyboardMouseOverlay::renderCtrlRow(ImDrawList* drawlist, ImVec2 screenPos) {
	int ctrlYPos = screenPos.y + ((BUTTON_SIZE + BUTTON_MARGIN) * 4);
	int ctrlWidth = BUTTON_SIZE + 16 + 16;
	drawlist->AddRectFilled(ImVec2(screenPos.x, ctrlYPos), ImVec2(screenPos.x + ctrlWidth, ctrlYPos + BUTTON_SIZE), getColor(inputMap.at("LeftControl").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(screenPos.x + BUTTON_MARGIN, ctrlYPos + (BUTTON_SIZE / 2) - BUTTON_MARGIN), COLOR_WHITE, "Ctrl");

	int altXPos = screenPos.x + ctrlWidth + BUTTON_SIZE + BUTTON_MARGIN;
	drawlist->AddRectFilled(ImVec2(altXPos, ctrlYPos), ImVec2(altXPos + ctrlWidth, ctrlYPos + BUTTON_SIZE), getColor(inputMap.at("LeftAlt").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(altXPos + (ctrlWidth / 2) - BUTTON_MARGIN, ctrlYPos + (BUTTON_SIZE / 2) - BUTTON_MARGIN), COLOR_WHITE, "Alt");


	int spaceXPos = altXPos + ctrlWidth + BUTTON_MARGIN;
	int spaceWidth = (BUTTON_SIZE * 4) + BUTTON_MARGIN;
	drawlist->AddRectFilled(ImVec2(spaceXPos, ctrlYPos), ImVec2(spaceXPos + spaceWidth, ctrlYPos + BUTTON_SIZE), getColor(inputMap.at("SpaceBar").pressed), 0, ImDrawCornerFlags_None);
	drawlist->AddText(ImVec2(spaceXPos + (spaceWidth / 2) - 15 - BUTTON_MARGIN, ctrlYPos + (BUTTON_SIZE / 2) - BUTTON_MARGIN), COLOR_WHITE, "Space");
}

void KeyboardMouseOverlay::keyPressHelper(ImDrawList* drawlist, const std::vector<std::string> &keys, ImVec2 startingPos) {
	for (int i = 0; i < keys.size(); i++) {
		const std::string& key = keys[i];
		ImVec2 position = ImVec2(startingPos.x + (i * BUTTON_SIZE) + (BUTTON_MARGIN * i), startingPos.y);
		drawlist->AddRectFilled(position, ImVec2(position.x + BUTTON_SIZE, position.y + BUTTON_SIZE), getColor(inputMap.at(key).pressed), 0, ImDrawCornerFlags_None);
		drawlist->AddText(ImVec2(position.x + (BUTTON_SIZE / 2) - BUTTON_MARGIN, position.y + (BUTTON_SIZE / 2) - BUTTON_MARGIN), COLOR_WHITE, inputMap.at(key).name.c_str());
	}
}

ImColor KeyboardMouseOverlay::getColor(bool isEnabled) {
	return isEnabled ? COLOR_GRAY : COLOR_BLACK;
}
