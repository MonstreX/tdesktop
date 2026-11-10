/*
This file is part of Voyager, a fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

namespace style {
struct ChatLayoutMetrics;
} // namespace style

namespace Ui {
struct ChatPaintContext;
} // namespace Ui

namespace HistoryView {

class Element;

// Where the sender userpic is shown for a group of messages.
enum class UserpicPlacement : uchar {
	StickyBottom, // Telegram: follows the last visible message of a group.
	GroupTop, // Shown once, next to the first message of a group.
};

// Describes how messages are laid out in the chat history.
// Each chat style is one entry in the styles table.
struct ChatLayout {
	QString id;
	bool bubbles = true;
	bool leftAligned = false;
	bool unlimitedWidth = false;
	bool nameForEveryone = false;
	UserpicPlacement userpic = UserpicPlacement::StickyBottom;

	// Time and read marks at the right edge, on the first line.
	bool timeInHeader = false;

	// Read marks are painted on the left side of the time.
	bool sendStateBeforeTime = false;

	// One background color for all reply and quote blocks.
	std::optional<QColor> quoteBackground;

	// Text color of the quoted message preview in reply blocks.
	std::optional<QColor> replyTextColor;

	// Replaces the default chat metrics, nullptr keeps them.
	const style::ChatLayoutMetrics *metrics = nullptr;
};

extern const char kOptionClassicChatLayout[];

// The layout is chosen once per run, changing it requires a restart.
[[nodiscard]] const ChatLayout &CurrentChatLayout();

// Applies the layout metrics and colors, call right after styles start.
void ApplyChatLayoutStyle();

// Outgoing messages use their own palette only inside bubbles.
[[nodiscard]] bool UseOutgoingPalette(not_null<const Element*> view);

// Top of the userpic for UserpicPlacement::GroupTop,
// std::nullopt if this message shows no userpic.
[[nodiscard]] std::optional<int> GroupTopUserpicTop(
	not_null<const Element*> view,
	int itemTop);

// Without bubbles a selected message is highlighted by its whole row.
void PaintRowSelection(
	QPainter &p,
	const Ui::ChatPaintContext &context,
	int width,
	int height);

} // namespace HistoryView
