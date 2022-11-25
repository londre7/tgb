.PHONY: all clean install uninstall

INSTALL_PATH=/root/tg_bot
MODULE_NAME=tg_bot

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

CPPVER=-std=c++17
CFLAGS=-c
INCLUDE_PATH=-I/usr/include -Iinclude -I$(mkfile_dir)
LIBRARY_PATH=-L/usr/lib64 -L/usr/lib64/mysql
LIBS=-lpthread -lssl -lcrypto -lmysqlclient -ldl

MKLIST=src/tg_bot.o json/dump.o json/error.o json/hashtable.o json/hashtable_seed.o json/load.o json/memory.o json/pack_unpack.o json/strbuffer.o json/strconv.o json/utf.o json/value.o src/http.o src/socket.o src/user.o src/utils.o src/botutils.o src/configs.o src/processing_commands.o src/threads.o src/callback.o src/smmysql.o src/botapi.o src/parser.o src/db.o src/smkeylist.o src/chat.o
	
all: tg_bot
	
clean: 
			rm -f src/*.o
			rm -f json/*.o
			
src/%.o: src/%.cpp
			g++ $(CPPVER) $(CFLAGS) $(INCLUDE_PATH) $(LIBRARY_PATH) -o $@ $<
json/%.o: json/%.cpp
			g++ $(CFLAGS) $(INCLUDE_PATH) $(LIBRARY_PATH) -o $@ $<
			
tg_bot: $(MKLIST)
			g++ $(CPPVER) -o $(MODULE_NAME) $(MKLIST) $(LIBS)
			
install:
			mkdir -p $(INSTALL_PATH)
			touch $(INSTALL_PATH)/bot.conf
			install ./$(MODULE_NAME) $(INSTALL_PATH)
			
uninstall:
			rm -rf $(INSTALL_PATH)/$(MODULE_NAME)