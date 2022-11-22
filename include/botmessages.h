// в этом файлике собрал системные сообщения, сообщения, которые отправляет бот и надписи кнопок
#ifndef TGBOT_BOTMESSAGES_HEADER
#define TGBOT_BOTMESSAGES_HEADER

// сообщения в консольку
#define SYSTEMMSG_WSA_ERROR               "WSAStartup init has failed"
#define SYSTEMMSG_LOADCONF_FAILED         "WARNING! Loading bot config file failed"
#define SYSTEMMSG_LOADCONF_DEFAULT_PARAMS "Token = %s, Timeout = %s"
#define SYSTEMMSG_LOADCONF_LOADED         "CONFIG LOADED: Token = %s, Timeout = %s"
#define SYSTEMMSG_CHECKBOT_FAILED         "Check bot state failed"
#define SYSTEMMSG_CHECKBOT_OK             "Bot is OK: id = %llu, %s, %s"
#define SYSTEMMSG_LOADLASTBOTSTATE_FAILED "WARNING! Loading last bot state failed"
#define SYSTEMMSG_UPD_THR_STARTED         "Started %d update threads"
#define SYSTEMMSG_PROC_UPD_STARTED        "Started update processing loop"
#define SYSTEMMSG_MYSQL_QUERY_FAILED      "Query failed"
#define SYSTEMMSG_UPD_USERINFO_FAILED     "Update user info has failed"
#define SYSTEMMSG_REGISTER_USR_FAILED     "Register new user has failed"
#define SYSTEMMSG_REGISTER_USR_SUCCESS    "Register new user (%s, %s, %s, %s)"
#define SYSTEMMSG_MYSQL_QUERY_FAILED      "Query failed"
#define SYSTEMMSG_RECV_CALLBACK_QUERY     "Callback query \"%s\" from \"%s\" (%s)"
#define SYSTEMMSG_RECV_CONTACT            "Received contact (%s, %s) from \"%s\" (%s)"
#define SYSTEMMSG_RECV_STICKER            "Received sticker (%s) from \"%s\" (%s)"
#define SYSTEMMSG_RECV_PHOTO              "Received photo (%llu sizes) from \"%s\" (%s)"
#define SYSTEMMSG_RECV_NONTEXTMSG         "Received non-text message from \"%s\" (%s)"
#define SYSTEMMSG_RECV_MESSAGE            "Received message from \"%s\" (%s): %s"
#define SYSTEMMSG_RECV_EDITED_MESSAGE     "Edited message (%llu) in chat \"%s\" from \"%s\" (%s): %s"
#define SYSTEMMSG_DUPLICATE_CALLBACK      "Duplicate callbacks (%s, %s)"
#define SYSTEMMSG_UNEXPECTED_CALLBACK     "Unexpected callback token \"%s\""
#define SYSTEMMSG_FINDTHR_START_FIND      "find_thr: start find by uid=%llu"
#define SYSTEMMSG_FINDTHR_START_CHAT      "find_thr: start new chat between user1=%llu and user2=%llu"

#define STR_DONATIONS_REQUSITS            "🔗 <b>Криптовалюта:</b>\n"\
                                          "ETH, USDT <code>0xC63ccE61D7cD81F137229D4cE0D6453c0AE2E655</code>"

// текстовые сообщения от бота
#define BOTMSG_INTERNAL_ERROR             "Не удалось завершить операцию. Внутренняя ошибка."
#define BOTMSG_USR_NO_STATE_FREE          "Недоступно. Сначала завершите текущую операцию!"
#define BOTMSG_IN_DEVELOP                 "Находится в разработке \\ud83d\\udd27"
#define BOTMSG_CMD_INVALID_PARAMS         "\\u274cОдин или несколько параметров заданы неверно!\n\n"\
                                          "<b>Синтаксис:</b>\n%s\n"\
                                          "<b>Примеры:</b>\n%s"
#define BOTMSG_CANCEL                     "🚫 Отменено"
#define BOTMSG_PERMISSION_DENIED          "\\u26d4\\ufe0f У Вас нет доступа к этой команде!"
#define BOTMSG_RECR_USER_CONTACT          "Номер телефона для обратной связи принят."
#define BOTMSG_RECR_OTHER_CONTACT         "Чужие контактные данные меня не интересуют."
#define BOTMSG_CMD_UNKNOWN                "\\u274c Неизвестная команда."
#define BOTMSG_CMD_START                  "Добро пожаловать, <b>%s</b>!\n"\
                                          "Здесь ты можешь пообщаться со случайно выбранным собеседником 🤗.\n"\
                                          "Во время диалога можно отправлять только обычные текстовые сообщения и стикеры.\n\n"\
                                          "Для поиска собеседника введи <b><i>/find</i></b>\n\n"\
                                          "Поддержать проект:\n%s"
#define BOTMSG_PRIVATE_POLICY             "Здесь когда-то будет политика конфиденциальности"
#define BOTMSG_NOTHING_CANCEL             "Нечего отменять :)"
#define BOTMSG_FIND_CANCEL                "Поиск отменён. Чтобы снова запустить поиск, введи <b><i>/find</i></b>"
#define BOTMSG_OUTSIDE_CHAT               "В настоящий момент вы не в диалоге)"
#define BOTMSG_STOP_CHAT_BY_ME            "Диалог завершён. Чтобы начать новый, введи <b><i>/find</i></b>"
#define BOTMSG_STOP_CHAT_BY_RECEPIENT     "Собеседник завершил диалог. Чтобы начать новый, введи <b><i>/find</i></b>"
#define BOTMSG_START_FIND                 "Поиск начат, и займёт какое-то время. Для отмены введи <b><i>/cancel</i></b>"
#define BOTMSG_START_CHAT                 "Чат начался! Для завершения диалога введите <b><i>/stop</i></b>. " \
                                          "Напоминаем, что для Вашей безопасности не рекомендуем оставлять какие-либо персональные данные. Приятного общения!"
#define BOTMSG_HELPMSG_FIND               "Для поиска собеседника набери <b><i>/find</i></b>"
#define BOTMSG_HELPMSG_CANCEL             "Чтобы отменить поиск, введи <b><i>/cancel</i></b>"
#define BOTMSG_USER_NOT_FOUND             "❌ Пользователь не найден!"
#define BOTMSG_SUCCESSFULL                "✅ Успешно"
#define BOTMSG_DENIED_MYSELF              "❌ Невозможно устанавливать этот флаг себе!"
#define BOTMSG_INPUT_UID                  "Введите <b>uid</b> или <b>username</b>. Для отмены введите <i>/cancel</i>"
#define BOTMSG_INPUT_NBIT                 "Введите номер флажка, или <i>/cancel</i> для отмены"
#define BOTMSG_INVALID_INPUT_NBIT         "❌ Неверно задан номер флажка. Повторите попытку, или введите <i>/cancel</i> для отмены"
#define BOTMSG_INPUT_VALUE                "Введите значение (0 или 1), или <i>/cancel</i> для отмены"
#define BOTMSG_INVALID_INPUT_VALUE        "❌ Неверно задано значение. Повторите попытку, или введите <i>/cancel</i> для отмены"
#define BOTMSG_LIMIT_FREE_CHATS           "Вы достигли ограничения по количеству бесплатных чатов в день. Чтобы продолжить, приобретите пакет, либо дождитесь завтра :)"

// названия INLINE кнопок
#define INLINEBTN_CAPTION_PRIVATE_POLICY  "Политика конфиденциальности"
#define INLINEBTN_CAPTION_CMDLIST         "Список команд"
#define INLINEBTN_CAPTION_FIND            "✈️ Искать"

// названия REPLY кнопок
#define REPLYBTN_CAPTION_FIND             INLINEBTN_CAPTION_FIND
#define REPLYBTN_CAPTION_CANCEL           "🚫 Отмена"
#define REPLYBTN_CAPTION_STOP             "⛔ Завершить"

#endif