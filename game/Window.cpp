#include "Window.hpp"

#include <SDL_keyboard.h>

const std::unordered_map<uint32_t, uint32_t> Window::_keymap{
	{ SDLK_UNKNOWN, Key_Unknown },
	{ SDLK_SPACE, Key_Space },
	{ SDLK_QUOTE, Key_Quote },
	{ SDLK_COMMA, Key_Comma },
	{ SDLK_MINUS, Key_Minus },
	{ SDLK_PERIOD, Key_Period },
	{ SDLK_SLASH, Key_Slash },
	{ SDLK_0, Key_0 },
	{ SDLK_1, Key_1 },
	{ SDLK_2, Key_2 },
	{ SDLK_3, Key_3 },
	{ SDLK_4, Key_4 },
	{ SDLK_5, Key_5 },
	{ SDLK_6, Key_6 },
	{ SDLK_7, Key_7 },
	{ SDLK_8, Key_8 },
	{ SDLK_9, Key_9 },
	{ SDLK_SEMICOLON, Key_Semicolon },
	{ SDLK_EQUALS, Key_Equal },
	{ SDLK_a, Key_A },
	{ SDLK_b, Key_B },
	{ SDLK_c, Key_C },
	{ SDLK_d, Key_D },
	{ SDLK_e, Key_E },
	{ SDLK_f, Key_F },
	{ SDLK_g, Key_G },
	{ SDLK_h, Key_H },
	{ SDLK_i, Key_I },
	{ SDLK_j, Key_J },
	{ SDLK_k, Key_K },
	{ SDLK_l, Key_L },
	{ SDLK_m, Key_M },
	{ SDLK_n, Key_N },
	{ SDLK_o, Key_O },
	{ SDLK_p, Key_P },
	{ SDLK_q, Key_Q },
	{ SDLK_r, Key_R },
	{ SDLK_s, Key_S },
	{ SDLK_t, Key_T },
	{ SDLK_u, Key_U },
	{ SDLK_v, Key_V },
	{ SDLK_w, Key_W },
	{ SDLK_x, Key_X },
	{ SDLK_y, Key_Y },
	{ SDLK_z, Key_Z },
	{ SDLK_LEFTBRACKET, Key_LBracket },
	{ SDLK_BACKSLASH, Key_Backslash },
	{ SDLK_RIGHTBRACKET, Key_RBracket },
	{ SDLK_BACKQUOTE, Key_GraveAccent },
	{ SDLK_ESCAPE, Key_Escape },
	{ SDLK_RETURN, Key_Enter },
	{ SDLK_TAB, Key_Tab },
	{ SDLK_BACKSPACE, Key_Backspace },
	{ SDLK_INSERT, Key_Insert },
	{ SDLK_DELETE, Key_Delete },
	{ SDLK_RIGHT, Key_Right },
	{ SDLK_LEFT, Key_Left },
	{ SDLK_DOWN, Key_Down },
	{ SDLK_UP, Key_Up },
	{ SDLK_PAGEUP, Key_PageUp },
	{ SDLK_PAGEDOWN, Key_PageDown },
	{ SDLK_HOME, Key_Home },
	{ SDLK_END, Key_End },
	{ SDLK_CAPSLOCK, Key_CapsLock },
	{ SDLK_SCROLLLOCK, Key_ScrollLock },
	{ SDLK_NUMLOCKCLEAR, Key_NumLock },
	{ SDLK_PRINTSCREEN, Key_PrintScreen },
	{ SDLK_PAUSE, Key_Pause },
	{ SDLK_F1, Key_F1 },
	{ SDLK_F2, Key_F2 },
	{ SDLK_F3, Key_F3 },
	{ SDLK_F4, Key_F4 },
	{ SDLK_F5, Key_F5 },
	{ SDLK_F6, Key_F6 },
	{ SDLK_F7, Key_F7 },
	{ SDLK_F8, Key_F8 },
	{ SDLK_F9, Key_F9 },
	{ SDLK_F10, Key_F10 },
	{ SDLK_F11, Key_F11 },
	{ SDLK_F12, Key_F12 },
	{ SDLK_F13, Key_F13 },
	{ SDLK_F14, Key_F14 },
	{ SDLK_F15, Key_F15 },
	{ SDLK_F16, Key_F16 },
	{ SDLK_F17, Key_F17 },
	{ SDLK_F18, Key_F18 },
	{ SDLK_F19, Key_F19 },
	{ SDLK_F20, Key_F20 },
	{ SDLK_F21, Key_F21 },
	{ SDLK_F22, Key_F22 },
	{ SDLK_F23, Key_F23 },
	{ SDLK_F24, Key_F24 },
	{ SDLK_KP_0, Key_Num0 },
	{ SDLK_KP_1, Key_Num1 },
	{ SDLK_KP_2, Key_Num2 },
	{ SDLK_KP_3, Key_Num3 },
	{ SDLK_KP_4, Key_Num4 },
	{ SDLK_KP_5, Key_Num5 },
	{ SDLK_KP_6, Key_Num6 },
	{ SDLK_KP_7, Key_Num7 },
	{ SDLK_KP_8, Key_Num8 },
	{ SDLK_KP_9, Key_Num9 },
	{ SDLK_KP_DECIMAL, Key_NumDecimal },
	{ SDLK_KP_DIVIDE, Key_NumDivide },
	{ SDLK_KP_MULTIPLY, Key_NumMultiply },
	{ SDLK_KP_MINUS, Key_NumSubtract },
	{ SDLK_KP_PLUS, Key_NumAdd },
	{ SDLK_KP_ENTER, Key_NumEnter },
	{ SDLK_KP_EQUALS, Key_NumEqual },
	{ SDLK_LSHIFT, Key_LShift },
	{ SDLK_LCTRL, Key_LCtrl },
	{ SDLK_LALT, Key_LAlt },
	{ SDLK_RSHIFT, Key_RShift },
	{ SDLK_RCTRL, Key_RCtrl },
	{ SDLK_RALT, Key_RAlt },
	{ SDLK_MENU, Key_Menu }
};

void fromSdl(const uint16_t& from, uint8_t* to) {
	if (from & KMOD_CTRL)
		*to |= SystemInterface::Mod_Ctrl;
	if (from & KMOD_SHIFT)
		*to |= SystemInterface::Mod_Shift;
	if (from & KMOD_ALT)
		*to |= SystemInterface::Mod_Alt;
	if (from & KMOD_CAPS)
		*to |= SystemInterface::Mod_Caps;
}

void Window::_recreateWindow(){
	if (_window)
		SDL_DestroyWindow(_window);

	uint32_t windowFlags = SDL_WINDOW_OPENGL;

	switch (_windowInfo.mode) {
		case Undecorated:
			windowFlags |= SDL_WINDOW_BORDERLESS;
			break;
		case Resizable:
			windowFlags |= SDL_WINDOW_RESIZABLE;
			break;
		case Fullscreen:
			windowFlags |= SDL_WINDOW_FULLSCREEN;
			break;
		case WindowFullscreen:
			windowFlags |= SDL_WINDOW_BORDERLESS;
			break;
		case Hidden:
			windowFlags |= SDL_WINDOW_HIDDEN;
			break;
	}

	if (!_context)
		windowFlags |= SDL_WINDOW_HIDDEN;

	uint32_t monitor = 0;
	glm::uvec2 size;

	if (_windowInfo.monitor < SDL_GetNumVideoDisplays())
		monitor = _windowInfo.monitor;

	if (_windowInfo.mode == Fullscreen || _windowInfo.mode == WindowFullscreen)
		size = _windowInfo.resolution;
	else
		size = _windowInfo.size;

	_window = SDL_CreateWindow(_windowInfo.title.c_str(), SDL_WINDOWPOS_CENTERED_DISPLAY(monitor), SDL_WINDOWPOS_CENTERED_DISPLAY(monitor), size.x, size.y, windowFlags);

	if (!_context) {
		_context = SDL_GL_CreateContext(_window);
		gladLoadGLLoader(SDL_GL_GetProcAddress);
	}

	SDL_GL_MakeCurrent(_window, _context);

	SDL_SetRelativeMouseMode((SDL_bool)_windowInfo.lockedCursor);

	CALL_SYSTEMS(_engine, SystemInterface::windowSize)(size);
	CALL_SYSTEMS(_engine, SystemInterface::framebufferSize)(size);
	CALL_SYSTEMS(_engine, SystemInterface::windowOpen)(true);
}

Window::Window(Engine& engine, const ConstructorInfo& constructorInfo) : _engine(engine), _constructorInfo(constructorInfo){
	INTERFACE_ENABLE(_engine, SystemInterface::initiate)(-1);
	INTERFACE_ENABLE(_engine, SystemInterface::update)(-1);
	INTERFACE_ENABLE(_engine, SystemInterface::lateUpdate)(-1);
}

Window::~Window(){
	if (_context)
		SDL_GL_DeleteContext(_context);

	if (_window)
		SDL_DestroyWindow(_window);

	SDL_Quit();
}

void Window::initiate(const std::vector<std::string>& args){
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, _constructorInfo.contextVersionMajor);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, _constructorInfo.contextVersionMinor);

	if (_constructorInfo.coreContex)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	if (_constructorInfo.debugContext)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	_recreateWindow();
}

void Window::update(double dt){
	SDL_Event e;
	decltype(_keymap)::const_iterator i;
	uint8_t mod;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {

		case SDL_QUIT:
			closeWindow();
			continue;

		case SDL_WINDOWEVENT:
			switch (e.window.event) {

			case SDL_WINDOWEVENT_RESIZED:
				CALL_SYSTEMS(_engine, SystemInterface::windowSize)(glm::uvec2(e.window.data1, e.window.data2));
				CALL_SYSTEMS(_engine, SystemInterface::framebufferSize)(glm::uvec2(e.window.data1, e.window.data2));
				continue;

			case SDL_WINDOWEVENT_ENTER:
				CALL_SYSTEMS(_engine, SystemInterface::cursorEnter)(true);
				continue;

			case SDL_WINDOWEVENT_LEAVE:
				CALL_SYSTEMS(_engine, SystemInterface::cursorEnter)(false);
				continue;
			}
			continue;

		case SDL_KEYDOWN:
			 i = _keymap.find(e.key.keysym.sym);
			fromSdl(e.key.keysym.mod, &mod);

			if (i != _keymap.end())
				CALL_SYSTEMS(_engine, SystemInterface::keyInput)(i->second, Action::Press, mod);
			else
				CALL_SYSTEMS(_engine, SystemInterface::keyInput)(Key_Unknown, Action::Press, mod);
			continue;

		case SDL_KEYUP:
			i = _keymap.find(e.key.keysym.sym);
			fromSdl(e.key.keysym.mod, &mod);

			if (i != _keymap.end())
				CALL_SYSTEMS(_engine, SystemInterface::keyInput)(i->second, Action::Release, mod);
			else
				CALL_SYSTEMS(_engine, SystemInterface::keyInput)(Key_Unknown, Action::Release, mod);
			continue;

		case SDL_MOUSEBUTTONDOWN:
			fromSdl(SDL_GetModState(), &mod);

			CALL_SYSTEMS(_engine, SystemInterface::mousePress)(e.button.button, Action::Press, mod);
			continue;

		case SDL_MOUSEBUTTONUP:
			fromSdl(SDL_GetModState(), &mod);

			CALL_SYSTEMS(_engine, SystemInterface::mousePress)(e.button.button, Action::Release, mod);
			continue;

		case SDL_MOUSEMOTION:
			CALL_SYSTEMS(_engine, SystemInterface::cursorPosition)(glm::dvec2(e.motion.xrel, e.motion.yrel));
			//CALL_SYSTEMS(_engine, SystemInterface::cursorPosition)(glm::dvec2(e.motion.x, e.motion.y));
			continue;
		}
	}
}

void Window::lateUpdate(double dt){
	if (!_window)
		return;

	SDL_GL_SwapWindow(_window);
}

void Window::openWindow(const WindowInfo& windowInfo){
	_windowInfo = windowInfo;

	openWindow();
}

void Window::openWindow(){
	_recreateWindow();
}

void Window::closeWindow(){
	if (!_window)
		return;

	CALL_SYSTEMS(_engine, SystemInterface::windowOpen)(false);

	SDL_DestroyWindow(_window);
	_window = nullptr;
}

void Window::setMode(Mode mode) {
	if (_windowInfo.mode == mode)
		return;

	_windowInfo.mode = mode;
	_recreateWindow();
}

void Window::setSize(glm::uvec2 size) {
	_windowInfo.size = size;

	if (_window)
		SDL_SetWindowSize(_window, size.x, size.y);
}

void Window::setResolution(glm::uvec2 resolution) {
	if (_windowInfo.resolution == resolution)
		return;

	_windowInfo.resolution = resolution;
	_recreateWindow();
}

void Window::setLockedCursor(bool locked) {
	if (_windowInfo.lockedCursor == locked)
		return;

	_windowInfo.lockedCursor = locked;

	if (_window) {
		SDL_SetRelativeMouseMode((SDL_bool)locked);

		int x, y;
		SDL_GetWindowSize(_window, &x, &y);

		SDL_WarpMouseInWindow(_window, x / 2, y / 2);
	}
}

void Window::setMonitor(uint32_t monitor) {
	_windowInfo.monitor = monitor;
	_recreateWindow();
}

uint32_t Window::getMonitorCount() const {
	return SDL_GetNumVideoDisplays();
}

glm::uvec2 Window::getMonitorResolution(uint32_t monitor) const {
	SDL_Rect display;
	SDL_GetDisplayBounds(monitor, &display);

	return { display.w, display.h };
}