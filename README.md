# relationalDBMS
This project implements a simple relational database management system.
It is written in C++ and is compiled and tested using Microsoft Visual C++ 2010 Express.

The input to your program will consist of pseudo-SQL commands whose formats are illustrated below:
CREATE table (field type{, field type})
INSERT INTO table (value{, value})
SELECT * FROM table1, table2 WHERE field1 = field2
SELECT field {, field} FROM table
SELECT * FROM table WHERE field = constant
EXIT
where field is the name of a field and type is either INT or STRING. All queries will start on a new line
and never exceed that line. Keywords will always be in upper case while table-names and field names will
always be lower case. Constant values will always be integer values or strings delimited by apostrophes
(single quotes). String sizes will never exceed 64 bytes.

The struture of the database is stored in a data dictionary. The dictionary has one entry per attribute and contains:
1. the name of the table containing the attribute,
2. the name of the attribute,
3. the attribute type (I or S), and
4. the name of file in which the table is stored.
The dictionary is written back to the disk at the end of the session.


