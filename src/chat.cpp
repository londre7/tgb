#include "tg_bot.h"

DB_Chat::DB_Chat(uint64_t _innerID, uint64_t _uid1, uint64_t _uid2, uint64_t _uid_stop, time_t _start_time, time_t _end_time, int _state) :
	innerID(_innerID),
	uid1(_uid1),
	uid2(_uid2),
	uid_stop(_uid_stop),
	start_time(_start_time),
	end_time(_end_time),
	state(_state)
{
}

DB_Chat::DB_Chat(uint64_t _innerID, uint64_t _uid1, uint64_t _uid2, time_t _start_time) :
	innerID(_innerID),
	uid1(_uid1),
	uid2(_uid2),
	uid_stop(0ull),
	start_time(_start_time),
	end_time(0),
	state(CHATSTATE_CHATING)
{
}

uint64_t CreateChat(uint64_t _uid1, uint64_t _uid2)
{
	return InsertToDBWithRetID
	(
		SMAnsiString::smprintf
		(
			"INSERT INTO tgb_chats (uid1, uid2, start_time) VALUES (%llu, %llu, %d)",
			_uid1,
			_uid2,
			(int)time(NULL)
		)
	);
}

DB_Chat* GetChatByInnerID(uint64_t _innerID)
{
	MySQLTablePtr chat_tbl(QueryFromDB(SMAnsiString::smprintf("SELECT * FROM tgb_chats WHERE id=%llu", _innerID)));
	return (chat_tbl && chat_tbl->Rows) ? new DB_Chat
	(
		_innerID,
		chat_tbl->Cell[0][1], // uid1
		chat_tbl->Cell[0][2], // uid2
		chat_tbl->Cell[0][3], // uid_stop
		chat_tbl->Cell[0][4], // start_time
		chat_tbl->Cell[0][5], // end_time
		chat_tbl->Cell[0][6]  // state
	) : nullptr;
}

size_t GetNumChatsForUser(uint64_t uid, time_t _from, time_t _to)
{
	if (!uid) return 0ull;
	if (_from && _to && (_from > _to)) return 0ull;

	SMAnsiString query = SMAnsiString::smprintf("SELECT COUNT(1) FROM tgb_chats WHERE (uid1=%llu OR uid2=%llu)", uid, uid);
	if (_from || _to) query += " AND (";
	if (_from) query += SMAnsiString::smprintf("start_time>=%d", (int)_from);
	if (_from && _to) query += " AND ";
	if (_to) query += SMAnsiString::smprintf("end_time<=%d", (int)_to);
	if (_from || _to) query += ")";
	query += " AND (state=1)";

	MySQLTablePtr tbl(QueryFromDB(query));
	return (tbl && tbl->Rows) ? (size_t)tbl->Cell[0][0] : 0ull;
}

bool StopChat(uint64_t _innerID, uint64_t _uid_stop)
{
	time_t end_time = time(NULL);
	return InsertToDB
	(
		SMAnsiString::smprintf
		(
			"UPDATE tgb_chats SET uid_stop = %llu, end_time = %d, state = %d WHERE id=%llu",
			_uid_stop,
			end_time,
			CHATSTATE_FINISHED,
			_innerID
		)
	);
}