/*
This file is part of Voyager, a fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings/settings_chat_layout.h"

#include "core/application.h"
#include "history/view/history_view_chat_layout.h"
#include "lang/lang_keys.h"
#include "settings/settings_builder.h"
#include "ui/boxes/confirm_box.h"
#include "ui/vertical_list.h"
#include "ui/widgets/checkbox.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"
#include "styles/style_settings.h"

namespace Settings {
namespace {

using namespace Builder;
using HistoryView::ChatLayoutId;

void SetupChatLayout(
		not_null<Window::SessionController*> controller,
		not_null<Ui::VerticalLayout*> container) {
	Ui::AddDivider(container);
	Ui::AddSkip(container);
	Ui::AddSubsectionTitle(container, tr::lng_settings_chat_layout());

	// The running layout stays until a restart, the choice is saved.
	const auto current = HistoryView::CurrentChatLayout().id;
	const auto group = std::make_shared<Ui::RadioenumGroup<ChatLayoutId>>(
		current);
	const auto add = [&](ChatLayoutId value, const QString &text) {
		container->add(
			object_ptr<Ui::Radioenum<ChatLayoutId>>(
				container,
				group,
				value,
				text,
				st::settingsSendType),
			st::settingsSendTypePadding);
	};
	add(
		ChatLayoutId::Telegram,
		tr::lng_settings_chat_layout_telegram(tr::now));
	add(
		ChatLayoutId::Classic,
		tr::lng_settings_chat_layout_classic(tr::now));
	add(ChatLayoutId::Qip, tr::lng_settings_chat_layout_qip(tr::now));

	group->setChangedCallback([=](ChatLayoutId value) {
		if (value == current) {
			return;
		}
		const auto confirmed = [=] {
			HistoryView::SaveChatLayout(value);
			Core::Restart();
		};
		const auto cancelled = [=](Fn<void()> close) {
			group->setValue(current);
			close();
		};
		controller->show(Ui::MakeConfirmBox({
			.text = tr::lng_settings_need_restart(),
			.confirmed = confirmed,
			.cancelled = cancelled,
			.confirmText = tr::lng_settings_restart_now(),
		}));
	});
	Ui::AddSkip(container);
}

} // namespace

void BuildChatLayoutSection(SectionBuilder &builder) {
	builder.add([](const WidgetContext &ctx) {
		SetupChatLayout(ctx.controller, ctx.container);
		return SectionBuilder::WidgetToAdd{};
	}, [] {
		return SearchEntry{
			.id = u"chat/chat-style"_q,
			.title = tr::lng_settings_chat_layout(tr::now),
			.keywords = { u"chat"_q, u"style"_q, u"layout"_q, u"qip"_q },
		};
	});
}

} // namespace Settings
