CREATE DATABASE OOP;
USE OOP;
CREATE TABLE qwe(id INT, aint INT, bchar CHAR, cdouble DOUBLE, PRIMARY KEY(id));

INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 1,      11, 'a',     100.0);
INSERT INTO qwe(id, aint, bchar         ) VALUES( 2, 4144959, 'b'           );
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 3,      13, 'c',     300.0);
INSERT INTO qwe(id, aint,        cdouble) VALUES( 4,      14,      4144959.0);
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 5,      15, 'e',     500.0);
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 6, 4144959, 'f', 4144959.0);
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 7,      17, 'g',     700.0);
INSERT INTO qwe(id,       bchar, cdouble) VALUES( 8,          'h', 4144959.0);
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 9,      19, 'i',     900.0);
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES(10,      20, 'j', 4144959.0);

SELECT * FROM qwe;
SELECT * FROM qwe WHERE aint > 50;
SELECT * FROM qwe WHERE cdouble > 1000.0;
SELECT bchar FROM qwe;
SELECT * FROM qwe WHERE aint = 4144959;

DROP DATABASE OOP;
