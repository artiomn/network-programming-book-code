#pragma once

#include <cinttypes>
#include <functional>
#include <optional>
#include <string>

#include <tgbot/tgbot.h>


typedef std::function<bool(TgBot::Bot &bot, int64_t chat_id, std::vector<std::string> &&command, const std::optional<int> &parameter)> CommandHandler;

void create_and_run_tg_bot(const std::string &token, CommandHandler on_command);

