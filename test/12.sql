CREATE DATABASE db;
USE db;
CREATE TABLE wtable1(key0 INT, key1 INT, tql_values INT NOT NULL, wtf DOUBLE, PRIMARY KEY(key1));
CREATE TABLE wtable2(key0 INT, key1 INT, tql_values INT NOT NULL, wtf DOUBLE, PRIMARY KEY(key1));
INSERT into wtable1(key0, key1, tql_values,wtf)values (134, 406, -20, -0.25);
INSERT into wtable1(key0, key1, tql_values,wtf)values (135, 416, -21, -1.25);
select * from wtable1;
