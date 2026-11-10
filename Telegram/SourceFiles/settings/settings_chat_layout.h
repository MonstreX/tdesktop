/*
This file is part of Voyager, a fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

namespace Settings::Builder {
class SectionBuilder;
} // namespace Settings::Builder

namespace Settings {

// "Chat style" choice: Telegram, Classic or QIP, applied after a restart.
void BuildChatLayoutSection(Builder::SectionBuilder &builder);

} // namespace Settings
