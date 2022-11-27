#ifndef TGBOT_THREADS_HEADER
#define TGBOT_THREADS_HEADER

#include "tg_bot.h"

class FindThread
{
	private:
		struct thr_par
		{
			pthread_mutex_t   m_UsrMutex;
			pthread_t         m_Thr;
			SMKeyList         m_Users;
			thr_par() : m_Users(32) {}
		} thrpar;

		inline void mutex_init();
		static void* thrFunc(void* arg);

	public:
		FindThread();
		void StartFind(uint64_t uid);
};
FindThread* GetFindThread();

int GetNumUpdThreads(void);
inline void FreeUpdateTreads();
void CreateUpdateThreads(int numthr);
void* UpdThreadFunc(void *arg);
bool PushToUpdQueue(TGBOT_Update* upd);

#endif