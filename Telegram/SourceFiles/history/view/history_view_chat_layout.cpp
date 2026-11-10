/*
This file is part of Voyager, a fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "history/view/history_view_chat_layout.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "history/history_item.h"
#include "history/view/history_view_element.h"
#include "ui/chat/chat_style.h"
#include "styles/style_chat.h"
#include "styles/style_history_view_chat_layout.h"

namespace HistoryView {
namespace {

constexpr auto kChatLayoutPref = "voyager_chat_layout";
constexpr auto kSendStateIconSize = 16;

enum class SendState : uchar {
	Incoming,
	Sending,
	Sent,
	Read,
	Failed,
};
constexpr auto kSendStateCount = 5;

[[nodiscard]] QByteArray SerializeChatLayoutId(ChatLayoutId id) {
	switch (id) {
	case ChatLayoutId::Telegram: return "telegram";
	case ChatLayoutId::Classic: return "classic";
	case ChatLayoutId::Qip: return "qip";
	}
	Unexpected("ChatLayoutId in SerializeChatLayoutId.");
}

[[nodiscard]] ChatLayoutId SavedChatLayoutId() {
	const auto value = Core::App().settings().readPref<QByteArray>(
		kChatLayoutPref);
	if (value == SerializeChatLayoutId(ChatLayoutId::Classic)) {
		return ChatLayoutId::Classic;
	} else if (value == SerializeChatLayoutId(ChatLayoutId::Qip)) {
		return ChatLayoutId::Qip;
	}
	return ChatLayoutId::Telegram;
}

[[nodiscard]] ChatLayout TelegramLayout() {
	return {};
}

[[nodiscard]] ChatLayout ClassicLayout() {
	return {
		.id = ChatLayoutId::Classic,
		.bubbles = false,
		.leftAligned = true,
		.unlimitedWidth = true,
		.nameForEveryone = true,
		.userpic = UserpicPlacement::GroupTop,
		.time = TimePlacement::HeaderRight,
		.sendState = SendStatePlacement::BeforeTime,
		.quoteBackground = QColor(0xF2, 0xF2, 0xF2),
		.replyTextColor = QColor(0x55, 0x55, 0x55),
		.metrics = &st::historyClassicMetrics,
	};
}

[[nodiscard]] ChatLayout QipLayout() {
	return {
		.id = ChatLayoutId::Qip,
		.bubbles = false,
		.leftAligned = true,
		.unlimitedWidth = true,
		.nameForEveryone = true,
		.headerForEachMessage = true,
		.userpic = UserpicPlacement::None,
		.time = TimePlacement::AfterName,
		.sendState = SendStatePlacement::BeforeName,
		.nameColoring = NameColoring::Direction,
		.incomingNameColor = QColor(0xD2, 0x48, 0x3E),
		.outgoingNameColor = QColor(0x2E, 0x86, 0xB0),
		.quoteBackground = QColor(0xF2, 0xF2, 0xF2),
		.replyTextColor = QColor(0x55, 0x55, 0x55),
		.metrics = &st::historyQipMetrics,
	};
}

[[nodiscard]] ChatLayout MakeChatLayout(ChatLayoutId id) {
	switch (id) {
	case ChatLayoutId::Telegram: return TelegramLayout();
	case ChatLayoutId::Classic: return ClassicLayout();
	case ChatLayoutId::Qip: return QipLayout();
	}
	Unexpected("ChatLayoutId in MakeChatLayout.");
}

[[nodiscard]] SendState ComputeSendState(not_null<const Element*> view) {
	const auto item = view->data();
	if (!view->hasOutLayout()) {
		return SendState::Incoming;
	} else if (item->hasFailed()) {
		return SendState::Failed;
	} else if (item->isSending()) {
		return SendState::Sending;
	} else if (view->delegate()->elementShownUnread(view)) {
		return SendState::Sent;
	}
	return SendState::Read;
}

[[nodiscard]] QString SendStateIconName(SendState state) {
	switch (state) {
	case SendState::Incoming: return u"status_incoming"_q;
	case SendState::Sending: return u"status_sending"_q;
	case SendState::Sent: return u"status_sent"_q;
	case SendState::Read: return u"status_read"_q;
	case SendState::Failed: return u"status_failed"_q;
	}
	Unexpected("SendState in SendStateIconName.");
}

// Icons come in 16px and 32px, the closest bigger one is scaled down.
[[nodiscard]] const QImage &SendStateIcon(SendState state) {
	static auto cache = std::array<QImage, kSendStateCount>();
	auto &result = cache[int(state)];
	if (result.isNull()) {
		const auto ratio = style::DevicePixelRatio();
		const auto size = style::ConvertScale(kSendStateIconSize) * ratio;
		const auto suffix = (size > kSendStateIconSize)
			? u"@2x.png"_q
			: u".png"_q;
		const auto path = u":/voyager/qip/"_q
			+ SendStateIconName(state)
			+ suffix;
		result = QImage(path).scaled(
			size,
			size,
			Qt::IgnoreAspectRatio,
			Qt::SmoothTransformation);
		result.setDevicePixelRatio(ratio);
	}
	return result;
}

} // namespace

const ChatLayout &CurrentChatLayout() {
	static const auto result = MakeChatLayout(SavedChatLayoutId());
	return result;
}

void SaveChatLayout(ChatLayoutId id) {
	auto &settings = Core::App().settings();
	settings.writePref<QByteArray>(kChatLayoutPref, SerializeChatLayoutId(id));
	Core::App().saveSettings();
}

void ApplyChatLayoutStyle() {
	const auto &layout = CurrentChatLayout();
	if (const auto metrics = layout.metrics) {
		// Generated style values live in mutable storage behind const
		// references. A layout replaces these few chat metrics once,
		// right after the style manager has filled them.
		const_cast<style::margins&>(st::msgPadding) = metrics->padding;
		const_cast<style::margins&>(st::msgMargin) = metrics->margin;
		const_cast<int&>(st::msgPhotoSkip) = metrics->photoSkip;
	}
	Ui::SetQuoteBackgroundOverride(layout.quoteBackground);
}

bool UseOutgoingPalette(not_null<const Element*> view) {
	return view->hasOutLayout() && CurrentChatLayout().bubbles;
}

std::optional<QColor> DirectionNameColor(not_null<const Element*> view) {
	const auto &layout = CurrentChatLayout();
	if (layout.nameColoring != NameColoring::Direction) {
		return std::nullopt;
	}
	return view->hasOutLayout()
		? layout.outgoingNameColor
		: layout.incomingNameColor;
}

std::optional<int> GroupTopUserpicTop(
		not_null<const Element*> view,
		int itemTop) {
	if (!view->hasFromPhoto() || view->isAttachedToPrevious()) {
		return std::nullopt;
	}
	// Center the userpic on the sender name and the first text line.
	const auto contentTop = itemTop + view->marginTop();
	const auto linesHeight = view->displayFromName()
		? (st::msgNameFont->height + st::msgFont->height)
		: st::msgFont->height;
	const auto centered = contentTop
		+ st::msgPadding.top()
		+ (linesHeight - st::msgPhotoSize) / 2;
	return std::max(centered, contentTop);
}

int NameSendStateSkip() {
	const auto &layout = CurrentChatLayout();
	if (layout.sendState != SendStatePlacement::BeforeName) {
		return 0;
	}
	const auto gap = layout.metrics ? layout.metrics->sendStateSkip : 0;
	return style::ConvertScale(kSendStateIconSize) + gap;
}

void PaintNameSendState(
		QPainter &p,
		not_null<const Element*> view,
		int left,
		int nameTop) {
	if (!NameSendStateSkip()) {
		return;
	}
	const auto size = style::ConvertScale(kSendStateIconSize);
	const auto top = nameTop + (st::msgNameFont->height - size) / 2;
	p.drawImage(left, top, SendStateIcon(ComputeSendState(view)));
}

QString NameTimeText(not_null<const Element*> view) {
	if (CurrentChatLayout().time != TimePlacement::AfterName) {
		return QString();
	}
	return '(' + view->dateTime().toString(u"hh:mm:ss dd/MM/yyyy"_q) + ')';
}

void PaintRowSelection(
		QPainter &p,
		const Ui::ChatPaintContext &context,
		int width,
		int height) {
	if (context.selected()) {
		p.fillRect(0, 0, width, height, context.st->msgSelectOverlay());
	}
}

} // namespace HistoryView
