-- moudle  : client encryption
-- purpose : function test
-- detail  : test CREATE AS & SELECT INTO (with encrypted columns)

-- (0) prepare | clean environment | succeed
CREATE SCHEMA ce_crt_tbl_as;
SET search_path TO ce_crt_tbl_as;

DROP TABLE IF EXISTS t1;
DROP TABLE IF EXISTS t2;
DROP CLIENT MASTER KEY IF EXISTS cmk1 CASCADE;
DROP CLIENT MASTER KEY IF EXISTS cmk2 CASCADE;
\! gs_ktool -d all

-- (0) prepare | create cmk & cek | succeed
\! gs_ktool -g
\! gs_ktool -g

CREATE CLIENT MASTER KEY cmk1 WITH (KEY_STORE = gs_ktool, KEY_PATH = "gs_ktool/1" , ALGORITHM = AES_256_CBC);
CREATE CLIENT MASTER KEY cmk2 WITH (KEY_STORE = gs_ktool, KEY_PATH = "gs_ktool/2" , ALGORITHM = AES_256_CBC);

CREATE COLUMN ENCRYPTION KEY cek1 WITH VALUES (CLIENT_MASTER_KEY = cmk1, ALGORITHM = AEAD_AES_256_CBC_HMAC_SHA256);
CREATE COLUMN ENCRYPTION KEY cek2 WITH VALUES (CLIENT_MASTER_KEY = cmk2, ALGORITHM = AEAD_AES_256_CBC_HMAC_SHA256);
CREATE COLUMN ENCRYPTION KEY cek3 WITH VALUES (CLIENT_MASTER_KEY = cmk2, ALGORITHM = AEAD_AES_256_CBC_HMAC_SHA256, ENCRYPTED_VALUE = '1234567890abcdef1234567890abcde');

-- (0) prepare | create table & insert data | succeed
CREATE TABLE IF NOT EXISTS t1 (
    c1 INT, 
    c2 INT ENCRYPTED WITH (COLUMN_ENCRYPTION_KEY = cek1, ENCRYPTION_TYPE = DETERMINISTIC),
    c3 TEXT ENCRYPTED WITH (COLUMN_ENCRYPTION_KEY = cek2, ENCRYPTION_TYPE = DETERMINISTIC),
    c4 VARCHAR(20) ENCRYPTED WITH (COLUMN_ENCRYPTION_KEY = cek3, ENCRYPTION_TYPE = DETERMINISTIC));

CREATE TABLE IF NOT EXISTS t2 (
    c1 INT, 
    c2 INT ENCRYPTED WITH (COLUMN_ENCRYPTION_KEY = cek1, ENCRYPTION_TYPE = DETERMINISTIC),
    c3 TEXT ENCRYPTED WITH (COLUMN_ENCRYPTION_KEY = cek2, ENCRYPTION_TYPE = DETERMINISTIC),
    c4 VARCHAR(20) ENCRYPTED WITH (COLUMN_ENCRYPTION_KEY = cek3, ENCRYPTION_TYPE = DETERMINISTIC));

INSERT INTO t1 VALUES (1, 12, 'r1 c3', 'r1 c4');
INSERT INTO t1 VALUES (2, 22, 'r2 c3', 'r2 c4');
INSERT INTO t1 VALUES (3, 32, 'r3 c3', 'r3 c4');

-- INSERT INTO t2 (nothing)

-- (1) SELECT INTO | succeed
SELECT c1,c2       INTO t1_1 FROM t1;
SELECT c1,c3,c4    INTO t1_2 FROM t1;
SELECT c1,c2,c3,c4 INTO t1_3 FROM t1;
SELECT *           INTO t1_4 FROM t1;

SELECT c1,c4       INTO t1_5 FROM t1 WHERE c4 = 'r4 c4';
SELECT c1,c3,c4    INTO t1_6 FROM t1 WHERE c3 = 'r2 c3';
SELECT c1,c2,c4    INTO t1_7 FROM t1 WHERE c2 = 12 AND c4 = 'r1 c4';
SELECT *           INTO t1_8 FROM t1 WHERE c2 = 32;

SELECT c1,c2       INTO t2_1 FROM t2;
SELECT c1,c3,c4    INTO t2_2 FROM t2;
SELECT c1,c2,c3,c4 INTO t2_3 FROM t2;
SELECT *           INTO t2_4 FROM t2;

SELECT c1,c4       INTO t2_5 FROM t2 WHERE c4 = 'r4 c4';
SELECT c1,c3,c4    INTO t2_6 FROM t2 WHERE c3 = 'r2 c3';
SELECT c1,c2,c4    INTO t2_7 FROM t2 WHERE c2 = 12 AND c4 = 'r1 c4';
SELECT *           INTO t2_8 FROM t2 WHERE c2 = 32;

-- (2) SELECT INTO | no distribute column | failed 
-- but, in fastcheck, i can't undstand why they succeed
SELECT c2          INTO t1_20 FROM t1;
SELECT c2,c3       INTO t1_21 FROM t1;

SELECT c2          INTO t2_20 FROM t2;
SELECT c2,c3       INTO t2_21 FROM t2;

-- (3) SELECT INTO | result table alreay exist | failed
SELECT c1,c2       INTO t1_1 FROM t1;

SELECT c1,c2       INTO t2_1 FROM t2;

-- (4) show results | succeed
SELECT count(*) FROM gs_encrypted_columns;

SELECT * FROM t1_1 ORDER BY c1 ASC;
SELECT * FROM t1_2 ORDER BY c1 ASC;
SELECT * FROM t1_3 ORDER BY c1 ASC;
SELECT * FROM t1_4 ORDER BY c1 ASC;
SELECT * FROM t1_5 ORDER BY c1 ASC;
SELECT * FROM t1_6 ORDER BY c1 ASC;
SELECT * FROM t1_7 ORDER BY c1 ASC;
SELECT * FROM t1_8 ORDER BY c1 ASC;

SELECT * FROM t2_1 ORDER BY c1 ASC;
SELECT * FROM t2_2 ORDER BY c1 ASC;
SELECT * FROM t2_3 ORDER BY c1 ASC;
SELECT * FROM t2_4 ORDER BY c1 ASC;
SELECT * FROM t2_5 ORDER BY c1 ASC;
SELECT * FROM t2_6 ORDER BY c1 ASC;
SELECT * FROM t2_7 ORDER BY c1 ASC;
SELECT * FROM t2_8 ORDER BY c1 ASC;

-- (5) clean copyted table | succeed
DROP TABLE t1_1;
DROP TABLE t1_2;
DROP TABLE t1_3;
DROP TABLE t1_4;
DROP TABLE t1_5;
DROP TABLE t1_6;
DROP TABLE t1_7;
DROP TABLE t1_8;

DROP TABLE t2_1;
DROP TABLE t2_2;
DROP TABLE t2_3;
DROP TABLE t2_4;
DROP TABLE t2_5;
DROP TABLE t2_6;
DROP TABLE t2_7;
DROP TABLE t2_8;

DROP TABLE IF EXISTS t1_20;
DROP TABLE IF EXISTS t1_21;
DROP TABLE IF EXISTS t2_20;
DROP TABLE IF EXISTS t2_21;

-- (6) CREATE AS | succeed
CREATE TABLE t1_1 AS    SELECT c1,c2       FROM t1;
CREATE TABLE t1_2 AS    SELECT c1,c3,c4    FROM t1;
CREATE TABLE t1_3 AS    SELECT c1,c2,c3,c4 FROM t1;
CREATE TABLE t1_4 AS    SELECT *           FROM t1;

-- TODO : not support yet
CREATE TABLE t1_4 AS    SELECT c1,c4       FROM t1 WHERE c4 = 'r4 c4';
CREATE TABLE t1_5 AS    SELECT c1,c3,c4    FROM t1 WHERE c3 = 'r2 c3';
CREATE TABLE t1_6 AS    SELECT c1,c2,c4    FROM t1 WHERE c2 = 12 AND c4 = 'r1 c4';
CREATE TABLE t1_7 AS    SELECT *           FROM t1 WHERE c2 = 32;

CREATE TABLE t2_1 AS    SELECT c1,c2       FROM t1;
CREATE TABLE t2_2 AS    SELECT c1,c3,c4    FROM t1;
CREATE TABLE t2_3 AS    SELECT c1,c2,c3,c4 FROM t1;
CREATE TABLE t2_4 AS    SELECT *           FROM t1;

-- TODO : not support yet
CREATE TABLE t2_4 AS    SELECT c1,c4       FROM t2 WHERE c4 = 'r4 c4';
CREATE TABLE t2_5 AS    SELECT c1,c3,c4    FROM t2 WHERE c3 = 'r2 c3';
CREATE TABLE t2_6 AS    SELECT c1,c2,c4    FROM t2 WHERE c2 = 12 AND c4 = 'r1 c4';
CREATE TABLE t2_7 AS    SELECT *           FROM t2 WHERE c2 = 32;

-- (7) show results | succeed
SELECT count(*) FROM gs_encrypted_columns;

SELECT * FROM t1_1 ORDER BY c1 ASC;
SELECT * FROM t1_2 ORDER BY c1 ASC;
SELECT * FROM t1_3 ORDER BY c1 ASC;
SELECT * FROM t1_4 ORDER BY c1 ASC;
SELECT * FROM t1_5 ORDER BY c1 ASC;
SELECT * FROM t1_6 ORDER BY c1 ASC;
SELECT * FROM t1_7 ORDER BY c1 ASC;
SELECT * FROM t1_8 ORDER BY c1 ASC;

SELECT * FROM t2_1 ORDER BY c1 ASC;
SELECT * FROM t2_2 ORDER BY c1 ASC;
SELECT * FROM t2_3 ORDER BY c1 ASC;
SELECT * FROM t2_4 ORDER BY c1 ASC;
SELECT * FROM t2_5 ORDER BY c1 ASC;
SELECT * FROM t2_6 ORDER BY c1 ASC;
SELECT * FROM t2_7 ORDER BY c1 ASC;
SELECT * FROM t2_8 ORDER BY c1 ASC;

-- (8) clean copyted table
DROP TABLE t1_1;
DROP TABLE t1_2;
DROP TABLE t1_3;
DROP TABLE t1_4;

DROP TABLE t2_1;
DROP TABLE t2_2;
DROP TABLE t2_3;
DROP TABLE t2_4;

-- (9) finish | clean environment | succeed
DROP TABLE IF EXISTS t1;
DROP TABLE IF EXISTS t2;
DROP CLIENT MASTER KEY cmk1 CASCADE;
DROP CLIENT MASTER KEY cmk2 CASCADE;
\! gs_ktool -d all

-- should be empty
SELECT * FROM gs_encrypted_columns;

-- reset
RESET search_path;
 