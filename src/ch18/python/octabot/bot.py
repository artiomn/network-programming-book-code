import tempfile
import logging
import asyncio

from oct2py import Oct2Py, Oct2PyError
import config
from telebot.async_telebot import AsyncTeleBot

TELEBOT = AsyncTeleBot(config.token)
FORBIDDEN_COMMANDS = ["system"]
OCT_SESS_DICT = {}  # octave sessions storage
OCTAVE_TIMEOUT = 30  # Octave command evaluation timeout


def get_octave_session(chat_id):
    """ Create an octave session for the chat or get an existing one """
    octave_session = OCT_SESS_DICT.get(chat_id)
    if not octave_session:
        logging.info(f"Creating new octave shell for the chat {chat_id}")
        octave_session = Oct2Py()
        octave_session.eval("set(0, 'defaultfigurevisible', 'off');")
        OCT_SESS_DICT[chat_id] = octave_session
    return octave_session


@TELEBOT.message_handler(commands=['start', 'help'])
async def handle_start_help(message):
    help_message = f"A simple Octave interpreter bot. \n" \
                   f"Octave syntax - https://docs.octave.org/v6.3.0/Command-Syntax-and-Function-Syntax.html \n" \
                   f"The following commands are prohibited - '{', '.join(FORBIDDEN_COMMANDS)}' \n" \
                   f"Type 'help' to get octave help."
    await TELEBOT.send_message(message.chat.id, help_message)


def _octave_eval(octave_session, command):
    try:
        output = octave_session.eval(command, return_both=True, timeout=10)[0]
    except Oct2PyError as err:
        logging.error(err)
        output = f"Syntax error: {err}"
    logging.info(f"Answer for the user: {output}")
    return output


@TELEBOT.message_handler(content_types=["text"])
async def text_handler(message):
    command = message.text
    chat_id = message.chat.id
    octave_session = get_octave_session(chat_id)
    logging.info(f"Chat: {chat_id}, command: {command}")

    for forbidden_word in FORBIDDEN_COMMANDS:
        if forbidden_word in command.lower():
            logging.info(f"Prohibited word detected {forbidden_word}")
            await TELEBOT.send_message(message.chat.id, f"The use of '{forbidden_word}' is prohibited.")
            return

    if "plot" in command or "mesh" in command:
        with tempfile.NamedTemporaryFile(suffix='.jpg') as tmp_image_file:
            command = f"figure(1, 'visible', 'off'); \n" \
                      f"{command}\n  print -djpg  '{tmp_image_file.name}'; close(gcf)"
            logging.info(f"Modified command: {command}")
            output = _octave_eval(octave_session, command)
            if output:
                await TELEBOT.send_message(message.chat.id, output)
            with open(tmp_image_file.name, 'rb') as photo:
                await TELEBOT.send_photo(chat_id, photo)
    else:
        output = _octave_eval(octave_session, command)
        if output:
            await TELEBOT.send_message(message.chat.id, output)


async def main():
    await TELEBOT.polling(none_stop=True)


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    asyncio.run(main())
