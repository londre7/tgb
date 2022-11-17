#ifndef CHAT_HEADER
#define CHAT_HEADER

#include "tg_bot.h"

enum
{
	CHATSTATE_CHATING=0,
	CHATSTATE_FINISHED
};

class DB_Chat
{
	private:
		uint64_t innerID;

		uint64_t uid1;
		uint64_t uid2;
		uint64_t uid_stop;
		time_t start_time;
		time_t end_time;
		int state;

	public:
		explicit DB_Chat(uint64_t _innerID, uint64_t _uid1, uint64_t _uid2, uint64_t _uid_stop, time_t _start_time, time_t _end_time, int _state);
		explicit DB_Chat(uint64_t _innerID, uint64_t _uid1, uint64_t _uid2, time_t _start_time);

		// getters
		uint64_t GetInnerID() const { return innerID; }
		uint64_t GetUID1() const { return uid1; }
		uint64_t GetUID2() const { return uid2; }
		uint64_t GetUIDStop() const { return uid_stop; }
		time_t GetStartTime() const { return start_time; }
		time_t GetEndTime() const { return end_time; }
		int GetState() const { return state; }

};

uint64_t CreateChat(uint64_t _uid1, uint64_t _uid2);
DB_Chat* GetChatByInnerID(uint64_t _innerID);
size_t GetNumChatsForUser(uint64_t uid, time_t _from = 0, time_t _to = 0);
bool StopChat(uint64_t _innerID, uint64_t _uid_stop);

#endif