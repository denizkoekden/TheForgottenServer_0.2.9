CFLAGS = -I. -I/usr/include/libxml2 -I/usr/include/lua5.1

LIBLINK = -L/usr/lib -lxml2 -lpthread -llua5.1 -lboost_thread -lgmp -lmysqlclient -lboost_regex -lsqlite3 -llua5.1-sql-mysql -llua5.1-sql-sqlite3 -ldl -lboost_system

FLAGS = -D__NO_HOMEDIR_CONF__ -D__USE_MYSQL__

OBJ = account.o actions.o admin.o allocator.o ban.o baseevents.o creature.o creatureevent.o chat.o combat.o commands.o condition.o configmanager.o connection.o container.o cylinder.o database.o databasemysql.o databasesqlite.o depot.o fileloader.o game.o gui.o house.o housetile.o ioguild.o iologindata.o iomap.o iomapserialize.o inputbox.o item.o items.o logger.o luascript.o mailbox.o map.o md5.o monster.o monsters.o movement.o networkmessage.o npc.o otserv.o outfit.o outputmessage.o party.o player.o playerbox.o position.o protocol.o protocolgame.o protocollogin.o quests.o raids.o rsa.o scheduler.o scriptmanager.o server.o spawn.o spells.o status.o talkaction.o tasks.o teleport.o textlogger.o thing.o tile.o tools.o trashholder.o vocation.o waitlist.o weapons.o 

all: clean theforgottenserver

clean:
	rm -rf *.o

theforgottenserver: $(OBJ)
	g++ $(CFLAGS) $(FLAGS) -Wno-deprecated -O1 -o ./TheForgottenServer $(OBJ) $(LIBLINK)

    %.o:%.cpp
	g++ $(CFLAGS) $(FLAGS) -Wno-deprecated -O1 -c $+
