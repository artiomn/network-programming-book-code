#include "server_tg.h"

#include <algorithm>
// Need for tolower().
#include <cctype>
#include <exception>
#include <iostream>
#include <sstream>


using namespace TgBot;


template<class TKeyboardLayout>
void create_keyboard(const std::vector<std::vector<std::string>>& button_layout, typename TKeyboardLayout::Ptr& kb);


template<>
void create_keyboard<TgBot::InlineKeyboardMarkup>(const std::vector<std::vector<std::string>>& button_layout,
                                                  typename TgBot::InlineKeyboardMarkup::Ptr& kb)
{
    for (auto button_row : button_layout)
    {
        std::vector<typename TgBot::InlineKeyboardButton::Ptr> row;
        for (auto button_text : button_row)
        {
            TgBot::InlineKeyboardButton::Ptr button(new TgBot::InlineKeyboardButton);
            button->text = button_text;
            auto cb_data = button_text;
            std::transform(cb_data.begin(), cb_data.end(), cb_data.begin(), [](unsigned char c)
            {
                return std::isspace(c) ? '_' : std::tolower(c);
            });
            button->callbackData = button_text;
            std::cout
                << "Key added: "
                << button_text << " - "
                << cb_data
                << std::endl;
            row.push_back(button);
        }
        kb->inlineKeyboard.push_back(row);
    }
}


template<>
void create_keyboard<TgBot::ReplyKeyboardMarkup>(const std::vector<std::vector<std::string>>& button_layout,
                                                 typename TgBot::ReplyKeyboardMarkup::Ptr& kb)
{
    for (auto button_row : button_layout)
    {
        std::vector<typename TgBot::KeyboardButton::Ptr> row;
        for (auto button_text : button_row)
        {
            typename TgBot::KeyboardButton::Ptr button(new TgBot::KeyboardButton);
            button->text = button_text;
            row.push_back(button);
        }
        kb->keyboard.push_back(row);
    }
}


void run_tg_bot(TgBot::Bot &bot)
{
    std::cout
        << "Run Telegram bot"
        << std::endl;

    try
    {
        std::cout
            << "Bot username: "
            << bot.getApi().getMe()->username
            << std::endl;
        bot.getApi().deleteWebhook();

        TgLongPoll long_poll(bot);
        while (true)
        {
            std::cout << "Long poll cycle" << std::endl;
            long_poll.start();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Telegram bot error: " << e.what()
            << std::endl;
    }
}


void create_and_run_tg_bot(const std::string &token, CommandHandler on_command)
{
    ReplyKeyboardMarkup::Ptr keyboard(new ReplyKeyboardMarkup);

    // InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    // create_keyboard<InlineKeyboardMarkup>(

    create_keyboard<ReplyKeyboardMarkup>(
    {
        {"Rotate Shoulder", "Up Shoulder", "Down Shoulder"},
        {"Rotate Forearm", "Up Forearm", "Down Forearm"},
        {"Up Manipulator", "Down Manipulator", "Open Manipulator", "Close Manipulator"},
        {"Reboot", "Shutdown", "Status"}
    }, keyboard);

    std::cout
        << "Creating Telegram bot..."
        << std::endl;
    Bot bot(token);

    // For inline keyboard.
    /*
    bot.getEvents().onCommand("down_forearm", [&bot, &keyboard](Message::Ptr message)
    {
        std::string response = "test";
        bot.getApi().sendMessage(message->chat->id, response, false, 0, keyboard, "");
    });
    */

    std::vector<std::string> command;
    bool waiting_command_param = false;

    bot.getEvents().onAnyMessage([&bot, &keyboard, &waiting_command_param, &command, &on_command](Message::Ptr message)
    {
        std::string message_text = message->text;

        std::cout
            << "User wrote "
            << message_text
            << std::endl;

        auto updown_action_func = [&bot, &message, &command]() -> bool
        {
            if (StringTools::startsWith(message->text, "Up"))
            {
                command.push_back("up");
                bot.getApi().sendMessage(message->chat->id, "Enter up angle");
                return true;
            }
            else if (StringTools::startsWith(message->text, "Down"))
            {
                command.push_back("down");
                bot.getApi().sendMessage(message->chat->id, "Enter down angle");
                return true;
            }

            return false;
        };

        auto rotate_action_func = [&bot, &message, &command]() -> bool
        {
            if (StringTools::startsWith(message->text, "Rotate"))
            {
                command.push_back("rotate");
                bot.getApi().sendMessage(message->chat->id, "Enter rotation angle");
                return true;
            }

            return false;
        };

        if (StringTools::startsWith(message->text, "/start"))
        {
            bot.getApi().sendMessage(message->chat->id, "Bot started...", false, 0, keyboard);
            return;
        }

        if (StringTools::endsWith(message->text, "Shoulder"))
        {
            command.push_back("shoulder");
            if (updown_action_func())
            {
                waiting_command_param = true;
                return;
            }
            else if (rotate_action_func())
            {
                waiting_command_param = true;
                return;
            }
            else
            {
                command.clear();
                bot.getApi().sendMessage(message->chat->id, "Unknown shoulder command");
                return;
            }
        }
        else if (StringTools::endsWith(message->text, "Forearm"))
        {
            command.push_back("forearm");
            if (updown_action_func())
            {
                waiting_command_param = true;
                return;
            }
            else if (rotate_action_func())
            {
                waiting_command_param = true;
                return;
            }
            else
            {
                command.clear();
                bot.getApi().sendMessage(message->chat->id, "Unknown forearm command");
                return;
            }
        }
        else if (StringTools::endsWith(message->text, "Manipulator"))
        {
            command.push_back("manipulator");
            if (updown_action_func())
            {
                waiting_command_param = true;
                return;
            }
            else if (StringTools::startsWith(message_text, "Open"))
            {
                command.push_back("open");
                waiting_command_param = true;
                bot.getApi().sendMessage(message->chat->id, "Enter opening angle");
                return;
            }
            else if (StringTools::startsWith(message_text, "Close"))
            {
                command.push_back("close");
                waiting_command_param = true;
                bot.getApi().sendMessage(message->chat->id, "Enter closing angle");
                return;
            }
            else
            {
                command.clear();
                bot.getApi().sendMessage(message->chat->id, "Unknown manipulator command");
                return;
            }
        }
        else if ("Reboot" == message_text ||
                 "Status" == message_text ||
                 "Shutdown" == message_text)
        {
            if (!on_command)
            {
                command.clear();
                return;
            }

            std::transform(message_text.begin(), message_text.end(), message_text.begin(), [](unsigned char c)
            {
                return std::isspace(c) ? '_' : std::tolower(c);
            });

            std::stringstream ss;
            ss
                << message_text
                << " executing...";
            bot.getApi().sendMessage(message->chat->id, ss.str());

            command.push_back(message_text);
            if (on_command(bot, message->chat->id, std::move(command), std::nullopt))
            {
                bot.getApi().sendMessage(message->chat->id, "OK");
            }
            else
            {
                bot.getApi().sendMessage(message->chat->id, "Failed!");
            }

            command.clear();
            return;
        }
        else if (!waiting_command_param)
        {
            command.clear();
            bot.getApi().sendMessage(message->chat->id, "Unknown command: " + message_text);
            return;
        }

        if (waiting_command_param)
        {
            try
            {
                waiting_command_param = false;
                if (!on_command)
                {
                    command.clear();
                    return;
                }

                auto command_param = std::stoi(message_text);
                std::stringstream msg;

                msg
                    << "Command \"";
                for (const auto &c : command)
                    msg << c << " ";
                msg
                    << command_param
                    << "\" will be executed...";

                bot.getApi().sendMessage(message->chat->id, msg.str());
                if (on_command(bot, message->chat->id, std::move(command), command_param))
                {
                    bot.getApi().sendMessage(message->chat->id, "OK");
                }
                else
                {
                    bot.getApi().sendMessage(message->chat->id, "Failed!");
                }
                command.clear();
                return;
            }
            catch(const std::invalid_argument& e)
            {
                std::stringstream ss;
                ss
                    << "Invalid argument: "
                    << e.what();
                bot.getApi().sendMessage(message->chat->id, ss.str());
                command.clear();
            }
            catch(const std::out_of_range& e)
            {
                std::stringstream ss;
                ss
                    << "Out of Range: "
                    << e.what();
                bot.getApi().sendMessage(message->chat->id, ss.str());
                command.clear();
            }

            return;
        }

        waiting_command_param = false;
        command.clear();
        bot.getApi().sendMessage(message->chat->id, "Unknown command: " + message_text);
    });

    run_tg_bot(bot);
}

