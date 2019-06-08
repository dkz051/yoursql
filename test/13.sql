CREATE DATABASE dbno1;
USE dbno1;
CREATE TABLE wtable(key1 INT);
insert into wtable(key1) values (1);
update wtable SET key1 = -21 where key1 = 1;
select * from wtable;
