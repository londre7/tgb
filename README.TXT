Развёртывание Linux

В общем случае развёртывание проходит следующим образом:
- установить gcc, g++, make, openssl, mysql, библиотеки mysql, openssl;
- настроить Mysql;
- создать БД и таблицы;
- загрузить репозиторий;
- установить;
- внести данные для подключения к Telegram и к БД в bot.conf;
- запустить.

Установка зависимостей на Cent OS Stream 9

yum group install "Development Tools"
yum install openssl
dnf --enablerepo=crb install mysql-devel --nobest
dnf install epel-release --nobest
dnf install screen --nobest
wget https://dev.mysql.com/get/mysql80-community-release-el7-3.noarch.rpm
rpm -Uvh mysql80-community-release-el7-3.noarch.rpm
rpm --import https://repo.mysql.com/RPM-GPG-KEY-mysql-2022
yum install mysql-server
systemctl start mysqld.service
systemctl enable mysqld
ln -s /usr/lib64/mysql/libmysqlclient.so /usr/lib64/libmysqlclient.so

Установка зависимостей на CENT OS 7

yum group install "Development Tools"
yum install screen openssl mysql-devel
wget https://dev.mysql.com/get/mysql80-community-release-el7-3.noarch.rpm
rpm -Uvh mysql80-community-release-el7-3.noarch.rpm
rpm --import https://repo.mysql.com/RPM-GPG-KEY-mysql-2022
yum install mysql-server
systemctl start mysqld.service
systemctl enable mysqld
ln -s /usr/lib64/mysql/libmysqlclient.so /usr/lib64/libmysqlclient.so

Сборка и запуск бота

настраиваем Mysql:
grep 'password' /var/log/mysqld.log
mysql_secure_installation

запускаем Mysql:
mysql

если пароль root задавался во время настройки mysql:
mysql -u root --password=mypassword

проверить, какой метод аутентификации используют аккаунты пользователей MySQL с помощью следующей команды:
SELECT user,authentication_string,plugin,host FROM mysql.user;

если стоит auth-socket, то меняем на обычный пароль
ALTER USER 'root'@'localhost' IDENTIFIED WITH caching_sha2_password BY 'mypassword';
FLUSH PRIVILEGES;

создать таблицы из mysql.txt (достаточно скопировать оттуда текст и нажать Enter)

выходим из mysql:
exit;

для дальнейших заходов на Mysql
mysql -u root --password=mypassword

загружаем репозиторий и собираем:
git clone https://github.com/londre7/tgb.git tg_bot_src
cd tg_bot_src
make && make install

вносим токен для подключения к telegram и данные для подключения к БД:
nano /root/tg_bot/bot.conf

token=токен
num_update_threads=4
db_user=root
db_password=mypassword
db_database=tgbot

сохраняем
Поддерживаемые параметры для bot.conf описаны в include/configs.h

запуск бота:
screen -dmS tgbot /root/tg_bot/tg_bot

чтобы смотреть консольный вывод:
screen -r tgbot

выход из режима просмотра
Сначала комбинация Ctrl+A, затем клавиша D

завершить работу бота:
screen -r tgbot
нажать комбинацию Ctrl+C