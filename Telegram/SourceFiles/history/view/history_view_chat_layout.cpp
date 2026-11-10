/*
This file is part of Voyager, a fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "history/view/history_view_chat_layout.h"

#include "base/options.h"
#include "history/view/history_view_element.h"
#include "ui/chat/chat_style.h"
#include "styles/style_chat.h"
#include "styles/style_history_view_chat_layout.h"

namespace HistoryView {
namespace {

base::options::toggle ClassicChatLayoutOption({
	.id = kOptionClassicChatLayout,
	.name = "Classic chat layout",
	.description = "Show messages without bubbles, aligned to the left, "
		"with the sender name and userpic at the start of each group.",
	.restartRequired = true,
});

[[nodiscard]] ChatLayout TelegramLayout() {
	return { .id = u"telegram"_q };
}

[[nodiscard]] ChatLayout ClassicLayout() {
	return {
		.id = u"classic"_q,
		.bubbles = false,
		.leftAligned = true,
		.unlimitedWidth = true,
		.nameForEveryone = true,
		.userpic = UserpicPlacement::GroupTop,
		.timeInHeader = true,
		.sendStateBeforeTime = true,
		.quoteBackground = QColor(0xF2, 0xF2, 0xF2),
		.replyTextColor = QColor(0x55, 0x55, 0x55),
		.metrics = &st::historyClassicMetrics,
	};
}

} // namespace

const char kOptionClassicChatLayout[] = "classic-chat-layout";

const ChatLayout &CurrentChatLayout() {
	static const auto result = ClassicChatLayoutOption.value()
		? ClassicLayout()
		: TelegramLayout();
	return result;
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
