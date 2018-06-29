#include "stdafx.hpp"

#include "InputManager.hpp"

#include <assert.h>

#pragma warning(push, 0)
#include "imgui.h"
#pragma warning(pop)

#include "Logger.hpp"
#include "Window/Window.hpp"

namespace flex
{
	const real InputManager::MAX_JOYSTICK_ROTATION_SPEED = 15.0f;

	InputManager::InputManager()
	{
	}

	InputManager::~InputManager()
	{
	}

	void InputManager::Initialize(const GameContext& gameContext)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = (i32)KeyCode::KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = (i32)KeyCode::KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = (i32)KeyCode::KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = (i32)KeyCode::KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = (i32)KeyCode::KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = (i32)KeyCode::KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = (i32)KeyCode::KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = (i32)KeyCode::KEY_HOME;
		io.KeyMap[ImGuiKey_End] = (i32)KeyCode::KEY_END;
		io.KeyMap[ImGuiKey_Insert] = (i32)KeyCode::KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = (i32)KeyCode::KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = (i32)KeyCode::KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = (i32)KeyCode::KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = (i32)KeyCode::KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = (i32)KeyCode::KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = (i32)KeyCode::KEY_A; // for text edit CTRL+A: select all
		io.KeyMap[ImGuiKey_C] = (i32)KeyCode::KEY_C; // for text edit CTRL+C: copy
		io.KeyMap[ImGuiKey_V] = (i32)KeyCode::KEY_V; // for text edit CTRL+V: paste
		io.KeyMap[ImGuiKey_X] = (i32)KeyCode::KEY_X; // for text edit CTRL+X: cut
		io.KeyMap[ImGuiKey_Y] = (i32)KeyCode::KEY_Y; // for text edit CTRL+Y: redo
		io.KeyMap[ImGuiKey_Z] = (i32)KeyCode::KEY_Z; // for text edit CTRL+Z: undo

		m_ImGuiIniFilepathStr = RESOURCE_LOCATION + "config/imgui.ini";
		io.IniFilename  = m_ImGuiIniFilepathStr.c_str();

		ClearAllInputs(gameContext);
	}

	void InputManager::Update(const GameContext& gameContext)
	{
		// Keyboard keys
		for (auto iter = m_Keys.begin(); iter != m_Keys.end(); ++iter)
		{
			if (iter->second.down > 0)
			{
				++iter->second.down;
			}
		}

#if 0 // Log mouse states
		Logger::LogInfo("states:   ", false);
		for (u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
		{
			Logger::LogInfo(std::string((m_MouseButtonStates & (1 << i)) != 0 ? "1" : "0") + ", ", false);
		}
		Logger::LogNewLine();

		Logger::LogInfo("pressed:  ", false);
		for (u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
		{
			Logger::LogInfo(std::string((m_MouseButtonsPressed & (1 << i)) != 0 ? "1" : "0") + ", ", false);
		}
		Logger::LogNewLine();

		Logger::LogInfo("released: ", false);
		for (u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
		{
			Logger::LogInfo(std::string((m_MouseButtonsReleased & (1 << i)) != 0 ? "1" : "0") + ", ", false);
		}
		Logger::LogNewLine();
		Logger::LogNewLine();
#endif

		// Mouse buttons
		for (u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
		{
			m_MouseButtonsPressed  &= ~(1 << i);
			m_MouseButtonsReleased &= ~(1 << i);
			if (m_MouseButtonStates & (1 << i))
			{
				m_MouseButtonDrags[i].endLocation = m_MousePosition;
			}
		}

		// Gamepads
		for (i32 i = 0; i < 2; ++i)
		{
			real joystickX = GetGamepadAxisValue(i, GamepadAxis::RIGHT_STICK_X);
			real joystickY = GetGamepadAxisValue(i, GamepadAxis::RIGHT_STICK_Y);

			i32 currentQuadrant = -1;

			real minimumExtensionLength = 0.35f;
			real extensionLength = glm::length(glm::vec2(joystickX, joystickY));
			if (extensionLength > minimumExtensionLength)
			{
				if (m_GamepadStates[i].previousQuadrant != -1)
				{
					real currentAngle = atan2(joystickY, joystickX) + PI;
					real previousAngle = atan2(m_GamepadStates[i].pJoystickY, m_GamepadStates[i].pJoystickX) + PI;
					// Asymptote occurs on left
					if (joystickX < 0.0f)
					{
						if (m_GamepadStates[i].pJoystickY < 0.0f && joystickY >= 0.0f)
						{
							// CCW
							currentAngle -= TWO_PI;
						}
						else if (m_GamepadStates[i].pJoystickY >= 0.0f && joystickY < 0.0f)
						{
							// CW
							currentAngle += TWO_PI;
						}
					}

					real stickRotationSpeed = (currentAngle - previousAngle) / gameContext.deltaTime;
					stickRotationSpeed = glm::clamp(stickRotationSpeed,
													-MAX_JOYSTICK_ROTATION_SPEED,
													MAX_JOYSTICK_ROTATION_SPEED);

					m_GamepadStates[i].averageRotationSpeeds.AddValue(stickRotationSpeed);
				}

				if (joystickX > 0.0f)
				{
					currentQuadrant = (joystickY > 0.0f ? 2 : 1);
				}
				else
				{
					currentQuadrant = (joystickY > 0.0f ? 3 : 0);
				}
			}
			else
			{
				m_GamepadStates[i].averageRotationSpeeds.Reset();
			}

			m_GamepadStates[i].previousQuadrant = currentQuadrant;

			m_GamepadStates[i].pJoystickX = joystickX;
			m_GamepadStates[i].pJoystickY = joystickY;
		}
	}

	void InputManager::PostUpdate()
	{
		m_PrevMousePosition = m_MousePosition;
		m_ScrollXOffset = 0.0f;
		m_ScrollYOffset = 0.0f;
	}

	void InputManager::UpdateGamepadState(i32 gamepadIndex, real axes[6], u8 buttons[15])
	{
		assert(gamepadIndex == 0 || gamepadIndex == 1);

		for (i32 i = 0; i < 6; ++i)
		{
			m_GamepadStates[gamepadIndex].axes[i] = axes[i];
		}

		HandleRadialDeadZone(&m_GamepadStates[gamepadIndex].axes[0], &m_GamepadStates[gamepadIndex].axes[1]);
		HandleRadialDeadZone(&m_GamepadStates[gamepadIndex].axes[2], &m_GamepadStates[gamepadIndex].axes[3]);

		u32 pStates = m_GamepadStates[gamepadIndex].buttonStates;

		m_GamepadStates[gamepadIndex].buttonStates = 0;
		for (i32 i = 0; i < GAMEPAD_BUTTON_COUNT; ++i)
		{
			m_GamepadStates[gamepadIndex].buttonStates = m_GamepadStates[gamepadIndex].buttonStates | (buttons[i] << i);
		}

		u32 changedButtons = m_GamepadStates[gamepadIndex].buttonStates ^ pStates;
		m_GamepadStates[gamepadIndex].buttonsPressed = changedButtons & m_GamepadStates[gamepadIndex].buttonStates;
		m_GamepadStates[gamepadIndex].buttonsReleased = changedButtons & (~m_GamepadStates[gamepadIndex].buttonStates);
	}

	InputManager::GamepadState& InputManager::GetGamepadState(i32 gamepadIndex)
	{
		assert(gamepadIndex == 0 ||
			   gamepadIndex == 1);
		return m_GamepadStates[gamepadIndex];
	}

	// http://www.third-helix.com/2013/04/12/doing-thumbstick-dead-zones-right.html
	void InputManager::HandleRadialDeadZone(real* x, real* y)
	{
		real deadzone = 0.25f;

		glm::vec2 stick(*x, *y);
		real stickMagnitude = glm::length(stick);
		if (stickMagnitude < deadzone)
		{
			*x = 0.0f;
			*y = 0.0f;
		}
		else
		{
			glm::vec2 stickDir = (stick / stickMagnitude);
			glm::vec2 stickScaled = stickDir * ((stickMagnitude - deadzone) / (1.0f - deadzone));

			*x = stickScaled.x;
			*y = stickScaled.y;
		}
	}

	i32 InputManager::GetKeyDown(KeyCode keyCode, bool ignoreImGui) const
	{
		if (!ignoreImGui && ImGui::GetIO().WantCaptureKeyboard)
		{
			return 0;
		}

		auto value = m_Keys.find(keyCode);
		if (value != m_Keys.end())
		{
			return m_Keys.at(keyCode).down;
		}

		return 0;
	}

	bool InputManager::GetKeyPressed(KeyCode keyCode, bool ignoreImGui) const
	{
		return GetKeyDown(keyCode, ignoreImGui) == 1;
	}

	bool InputManager::IsGamepadButtonDown(i32 gamepadIndex, GamepadButton button)
	{
		assert(gamepadIndex == 0 || gamepadIndex == 1);

		bool down = ((m_GamepadStates[gamepadIndex].buttonStates & (1 << (i32)button)) != 0);
		return down;
	}

	bool InputManager::IsGamepadButtonPressed(i32 gamepadIndex, GamepadButton button)
	{
		assert(gamepadIndex == 0 || gamepadIndex == 1);

		bool pressed = ((m_GamepadStates[gamepadIndex].buttonsPressed & (1 << (i32)button)) != 0);
		return pressed;
	}

	bool InputManager::IsGamepadButtonReleased(i32 gamepadIndex, GamepadButton button)
	{
		assert(gamepadIndex == 0 || gamepadIndex == 1);

		bool released = ((m_GamepadStates[gamepadIndex].buttonsReleased & (1 << (i32)button)) != 0);
		return released;
	}

	real InputManager::GetGamepadAxisValue(i32 gamepadIndex, GamepadAxis axis)
	{
		assert(gamepadIndex == 0 || gamepadIndex == 1);
	
		real axisValue = m_GamepadStates[gamepadIndex].axes[(i32)axis];
		return axisValue;
	}

	void InputManager::CursorPosCallback(double x, double y)
	{
		m_MousePosition = glm::vec2((real)x, (real)y);

		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = m_MousePosition;
	}

	void InputManager::MouseButtonCallback(MouseButton mouseButton, Action action, i32 mods)
	{
		UNREFERENCED_PARAMETER(mods);

		assert((u32)mouseButton < MOUSE_BUTTON_COUNT);

		if (action == Action::PRESS)
		{
			m_MouseButtonStates    |=  (1 << (u32)mouseButton);
			m_MouseButtonsPressed  |=  (1 << (u32)mouseButton);
			m_MouseButtonsReleased &= ~(1 << (u32)mouseButton);

			m_MouseButtonDrags[(i32)mouseButton].startLocation = m_MousePosition;
			m_MouseButtonDrags[(i32)mouseButton].endLocation = m_MousePosition;
		}
		else if (action == Action::RELEASE)
		{
			m_MouseButtonStates    &= ~(1 << (u32)mouseButton);
			m_MouseButtonsPressed  &= ~(1 << (u32)mouseButton);
			m_MouseButtonsReleased |=  (1 << (u32)mouseButton);

			m_MouseButtonDrags[(i32)mouseButton].endLocation = m_MousePosition;
		}

		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[(i32)mouseButton] = (m_MouseButtonStates & (1 << (i32)mouseButton)) != 0;
	}

	void InputManager::ScrollCallback(double xOffset, double yOffset)
	{
		m_ScrollXOffset = (real)xOffset;
		m_ScrollYOffset = (real)yOffset;

		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheel = m_ScrollYOffset;
	}

	void InputManager::KeyCallback(KeyCode keycode, Action action, i32 mods)
	{
		UNREFERENCED_PARAMETER(mods);

		if (action == Action::PRESS)
		{
			m_Keys[keycode].down = 1;
		}
		else if (action == Action::REPEAT)
		{
			// Ignore repeat events (we're already counting how long the key is down for
		}
		else if (action == Action::RELEASE)
		{
			m_Keys[keycode].down = 0;
		}

		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[(i32)keycode] = m_Keys[keycode].down > 0;

		io.KeyCtrl = GetKeyDown(KeyCode::KEY_LEFT_CONTROL, true) || GetKeyDown(KeyCode::KEY_RIGHT_CONTROL, true);
		io.KeyShift = GetKeyDown(KeyCode::KEY_LEFT_SHIFT, true) || GetKeyDown(KeyCode::KEY_RIGHT_SHIFT, true);
		io.KeyAlt = GetKeyDown(KeyCode::KEY_LEFT_ALT, true) || GetKeyDown(KeyCode::KEY_RIGHT_ALT, true);
		io.KeySuper = GetKeyDown(KeyCode::KEY_LEFT_SUPER, true) || GetKeyDown(KeyCode::KEY_RIGHT_SUPER, true);
	}

	void InputManager::CharCallback(u32 character)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter((ImWchar)character);
	}

	glm::vec2 InputManager::GetMousePosition() const
	{
		return m_MousePosition;
	}

	void InputManager::SetMousePosition(glm::vec2 mousePos, bool updatePreviousPos)
	{
		m_MousePosition = mousePos;

		if (updatePreviousPos)
		{
			m_PrevMousePosition = m_MousePosition;
		}
	}

	glm::vec2 InputManager::GetMouseMovement() const
	{
		if (ImGui::GetIO().WantCaptureMouse)
		{
			return glm::vec2(0, 0);
		}

		return m_MousePosition - m_PrevMousePosition;
	}

	bool InputManager::GetMouseButtonDown(MouseButton mouseButton) const
	{
		if (ImGui::GetIO().WantCaptureMouse)
		{
			return 0;
		}

		assert((i32)mouseButton >= 0 && (i32)mouseButton <= MOUSE_BUTTON_COUNT - 1);

		return (m_MouseButtonStates & (1 << (i32)mouseButton)) != 0;
	}

	bool InputManager::GetMouseButtonPressed(MouseButton mouseButton) const
	{
		if (ImGui::GetIO().WantCaptureMouse)
		{
			return 0;
		}

		assert((i32)mouseButton >= 0 && (i32)mouseButton <= MOUSE_BUTTON_COUNT - 1);

		return (m_MouseButtonsPressed & (1 << (i32)mouseButton)) != 0;
	}

	bool InputManager::GetMouseButtonReleased(MouseButton mouseButton) const
	{
		if (ImGui::GetIO().WantCaptureMouse)
		{
			return 0;
		}

		assert((i32)mouseButton >= 0 && (i32)mouseButton <= MOUSE_BUTTON_COUNT - 1);

		return (m_MouseButtonsReleased & (1 << (i32)mouseButton)) != 0;
	}

	real InputManager::GetVerticalScrollDistance() const
	{
		if (ImGui::GetIO().WantCaptureMouse)
		{
			return 0.0f;
		}

		return m_ScrollYOffset;
	}

	glm::vec2 InputManager::GetMouseDragDistance(MouseButton mouseButton)
	{
		assert((i32)mouseButton >= 0 && (i32)mouseButton <= MOUSE_BUTTON_COUNT - 1);

		return (m_MouseButtonDrags[(i32)mouseButton].endLocation - m_MouseButtonDrags[(i32)mouseButton].startLocation);
	}

	void InputManager::ClearAllInputs(const GameContext& gameContext)
	{
		ClearMouseInput(gameContext);
		ClearKeyboadInput();
		ClearGampadInput(0);
		ClearGampadInput(1);
	}

	void InputManager::ClearMouseInput(const GameContext& gameContext)
	{
		m_PrevMousePosition = m_MousePosition;
		m_ScrollXOffset = 0.0f;
		m_ScrollYOffset = 0.0f;

		m_MouseButtonStates = 0;
		m_MouseButtonsPressed = 0;
		m_MouseButtonsReleased = 0;

		for (i32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
		{
			m_MouseButtonDrags[i].startLocation = glm::vec2(0.0f);
			m_MouseButtonDrags[i].endLocation = glm::vec2(0.0f);
		}
		gameContext.window->SetCursorMode(Window::CursorMode::NORMAL);


		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = m_MousePosition;
		io.MousePosPrev = m_PrevMousePosition;
		io.MouseWheel = m_ScrollYOffset;
	}

	void InputManager::ClearKeyboadInput()
	{
		for (auto iter = m_Keys.begin(); iter != m_Keys.end(); ++iter)
		{
			iter->second.down = 0;
		}
	}

	void InputManager::ClearGampadInput(i32 gamepadIndex)
	{
		for (i32 j = 0; j < 6; ++j)
		{
			m_GamepadStates[gamepadIndex].axes[j] = 0;
		}

		m_GamepadStates[gamepadIndex].buttonStates = 0;
		m_GamepadStates[gamepadIndex].buttonsPressed = 0;
		m_GamepadStates[gamepadIndex].buttonsReleased = 0;

		m_GamepadStates[gamepadIndex].averageRotationSpeeds = 
			RollingAverage(m_GamepadStates[gamepadIndex].framesToAverageOver);
	}
} // namespace flex
