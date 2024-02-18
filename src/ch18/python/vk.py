#!/usr/bin/env python3

import vk_api


vk_session = vk_api.VkApi('+71234567890', 'mypassword')
vk_session.auth()

vk = vk_session.get_api()
print(vk.wall.post(message='Hello world!'))
