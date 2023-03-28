# Описание схемы. Используется Cerberus для разбора и проверки грамматики.
people_schema = {
    # Поле 1 - имя.
    'firstname': {
        # Тип поля - строка.
        'type': 'string',
        'minlength': 1,
        'maxlength': 10,
    },
    # Поле 2 - фамилия.
    'lastname': {
        'type': 'string',
        'minlength': 1,
        'maxlength': 15,
        # Поле должно обязательно присутствовать в запросе.
        'required': True,
        # Введём ограничение: фамилия должна быть уникальна.
        'unique': True,
    },
}

# Домен - это ресурс верхнего уровня.
DOMAIN = {
    # Ресурс будет доступен, как hostname/people.
    'people': {
        # Тэг 'title' используется в ссылках на ресурс.
        'item_title': 'person',
        # Управление кэшированием.
        'cache_control': 'max-age=10,must-revalidate',
        'cache_expires': 10,
        # Добавить возможность получать ресурсы по фамилии.
        # GET 'host/people/<lastname>'.
        'additional_lookup': {'url': r'regex("[\w]+")', 'field': 'lastname'},
        # Разрешённые методы.
        'resource_methods': ['GET', 'POST'],
        # Подключить схему.
        'schema': people_schema,
    }
}
