#pragma once

#include <SDL.h>
#include <vector>
#include <string>
#include "core/Var.h"
#include "TurboBadger.h"

namespace ui {

class Console {
private:
	typedef std::vector<std::string> Messages;
	typedef Messages::const_reverse_iterator MessagesIter;
	Messages _messages;
	Messages _history;
	uint32_t _historyPos = 0;
	bool _consoleActive = false;
	SDL_LogOutputFunction _logFunction = nullptr;
	core::VarPtr _autoEnable;
	tb::TBFontFace *_font;
	std::string _commandLine;
	// commandline character will get overwritten if this is true
	bool _overwrite = false;
	bool _cursorBlink = false;
	int _frame = 0;
	int _cursorPos = 0;
	int _scrollPos = 0;
	int _maxLines = 0;

	static void logConsole(void *userdata, int category, SDL_LogPriority priority, const char *message);

	// cursor move on the commandline
	void cursorLeft();
	void cursorRight();

	// history 'scroll' methods
	void cursorUp();
	void cursorDown();

	void scrollUp();
	void scrollDown();

	void autoComplete();

	void executeCommandLine();

	// removed the character under the cursor position
	void cursorDelete(bool moveCursor = true);

public:
	Console();
	bool init();
	bool toggle();
	void render(const tb::TBRect &rect);
	bool isActive() const;
	bool onTextInput(const std::string& text);
	bool onKeyPress(int32_t key, int16_t modifier);
	bool onMouseWheel(int32_t x, int32_t y);
};

inline bool Console::isActive() const {
	return _consoleActive;
}

}
