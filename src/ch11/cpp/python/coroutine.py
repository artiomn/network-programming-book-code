import asyncio


async def async_func():
    print('Запуск ...')
    await asyncio.sleep(1)
    print('... Готово!')


async def main():
    task = asyncio.create_task (async_func())
    await task

asyncio.run(main())
