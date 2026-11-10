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

enum class ChatLayoutId : uchar {
	Telegram,
	Classic,
	Qip,
};

// Where the sender userpic is shown for a group of messages.
enum class UserpicPlacement : uchar {
	StickyBottom, // Telegram: follows the last visible message of a group.
	GroupTop, // Shown once, next to the first message of a group.
	None,
};

// Where the message time is shown.
enum class TimePlacement : uchar {
	InBubble, // Telegram: bottom right corner of the message.
	HeaderRight, // Right edge of the first line.
	AfterName, // "(hh:mm:ss dd/MM/yyyy)" right after the sender name.
};

// Where the send state (sending, sent, read) is shown.
enum class SendStatePlacement : uchar {
	AfterTime, // Telegram: check marks on the right of the time.
	BeforeTime, // Check marks on the left of the time.
	BeforeName, // An envelope icon on the left of the sender name.
};

// How the sender name is colored.
enum class NameColoring : uchar {
	Peer, // Telegram: the color of the sender.
	Direction, // One color for incoming and one for outgoing messages.
};

// Describes how messages are laid out in the chat history.
// Each chat style is one entry in the styles table.
struct ChatLayout {
	ChatLayoutId id = ChatLayoutId::Telegram;
	bool bubbles = true;
	bool leftAligned = false;
	bool unlimitedWidth = false;
	bool nameForEveryone = false;

	// Every message has its own header, even inside a group.
	bool headerForEachMessage = false;

	UserpicPlacement userpic = UserpicPlacement::StickyBottom;
	TimePlacement time = TimePlacement::InBubble;
	SendStatePlacement sendState = SendStatePlacement::AfterTime;
	NameColoring nameColoring = NameColoring::Peer;
	QColor incomingNameColor;
	QColor outgoingNameColor;

	// One background color for all reply and quote blocks.
	std::optional<QColor> quoteBackground;

	// Text color of the quoted message preview in reply blocks.
	std::optional<QColor> replyTextColor;

	// Replaces the default chat metrics, nullptr keeps them.
	const style::ChatLayoutMetrics *metrics = nullptr;
};

// The layout is chosen once per run, changing it requires a restart.
[[nodiscard]] const ChatLayout &CurrentChatLayout();

// Saves the layout to use after the next start.
void SaveChatLayout(ChatLayoutId id);

// Applies the layout metrics and colors, call right after styles start.
void ApplyChatLayoutStyle();

// Outgoing messages use their own palette only inside bubbles.
[[nodiscard]] bool UseOutgoingPalette(not_null<const Element*> view);

// Sender name color, std::nullopt keeps the Telegram color.
[[nodiscard]] std::optional<QColor> DirectionNameColor(
	not_null<const Element*> view);

// Top of the userpic for UserpicPlacement::GroupTop,
// std::nullopt if this message shows no userpic.
[[nodiscard]] std::optional<int> GroupTopUserpicTop(
	not_null<const Element*> view,
	int itemTop);

// Width taken by the send state icon before the sender name,
// zero if the layout shows no such icon.
[[nodiscard]] int NameSendStateSkip();

// Paints the send state icon centered on the sender name line.
void PaintNameSendState(
	QPainter &p,
	not_null<const Element*> view,
	int left,
	int nameTop);

// "(hh:mm:ss dd/MM/yyyy)" shown after the sender name,
// an empty string if the layout shows the time elsewhere.
[[nodiscard]] QString NameTimeText(not_null<const Element*> view);

// Without bubbles a selected message is highlighted by its whole row.
void PaintRowSelection(
	QPainter &p,
	const Ui::ChatPaintContext &context,
	int width,
	int height);

} // namespace HistoryView
