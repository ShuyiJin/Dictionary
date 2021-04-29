drop table if exists user_info;
drop table if exists history;

create table user_info(username varchar[32], password varchar[32]);
create table history(username varchar[32], word varchar[32], time varchar[32]);

