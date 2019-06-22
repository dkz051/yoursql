CREATE DATABASE OOP;
USE OOP;
CREATE TABLE qwe(id INT, aint INT, bchar CHAR, cdouble DOUBLE, PRIMARY KEY(id));

INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 1,      11, 'c',     100.0);
INSERT INTO qwe(id, aint, bchar         ) VALUES( 2, 4144958, 'b'           );
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 3,      11, 'c',     100.0);
INSERT INTO qwe(id, aint,        cdouble) VALUES( 4,      14,      4144958.0);
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 5,      11, 'a',     100.0);
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 6, 4144958, 'b', 4144958.0);
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 7,      17, 'c',     100.0);
INSERT INTO qwe(id,       bchar, cdouble) VALUES( 8,          'b', 4144958.0);
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES( 9,      17, 'a',     900.0);
INSERT INTO qwe(id, aint, bchar, cdouble) VALUES(10,      20, 'c', 4144958.0);

SELECT * FROM qwe ORDER BY aint DESC;
SELECT * FROM qwe ORDER BY aint DESC, cdouble, bchar ASC;

DROP DATABASE OOP;
