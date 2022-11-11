Поддерживаемые параметры для bot.conf:
описание представлено в include/configs.h

Развёртывание Linux

В общем случае развёртывание проходит следующим образом:
- установить gcc, g++, make, openssl, mysql, библиотеки mysql, openssl;
- настроить Mysql;
- создать БД и таблицы;
- загрузить репозиторий;
- установить;
- внести данные для подключения к Telegram и к БД в bot.conf;
- запустить.

Развёртывание для CENT OS 7

ставим всё необходимое:
yum group install "Development Tools"
yum install screen openssl mysql-devel
wget https://dev.mysql.com/get/mysql80-community-release-el7-3.noarch.rpm
rpm -Uvh mysql80-community-release-el7-3.noarch.rpm
rpm --import https://repo.mysql.com/RPM-GPG-KEY-mysql-2022
yum install mysql-server
systemctl start mysqld.service
systemctl enable mysqld
ln -s /usr/lib64/mysql/libmysqlclient.so /usr/lib64/libmysqlclient.so

настраиваем Mysql
grep 'password' /var/log/mysqld.log
mysql_secure_installation

запускаем Mysql
mysql

проверить, какой метод аутентификации используют аккаунты пользователей MySQL с помощью следующей команды:
SELECT user,authentication_string,plugin,host FROM mysql.user;

есть стоит auth-socket, то меняем на обычный пароль
ALTER USER 'root'@'localhost' IDENTIFIED WITH caching_sha2_password BY 'mypassword';
FLUSH PRIVILEGES;

создать таблицы из mysql.txt

выходим из mysql:
exit;

для дальнейших заходов на Mysql
mysql -u root --password=mypassword

загружаем репозиторий и собираем:
git clone https://github.com/londre7/tgb.git tg_bot
cd tg_bot
make && make install

вносим токен для подключения к telegram и данные для подключения к БД:
nano /root/tg_bot/bot.conf

token=токен
num_update_threads=4
db_user=root
db_password=mypassword
db_database=tgbot

сохраняем

запуск бота:
screen -dmS tgbot /root/tg_bot/tg_bot
screen -r tgbot
