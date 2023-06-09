\! gs_ktool -d all
\! gs_ktool -g

DROP CLIENT MASTER KEY IF EXISTS desc_cmk CASCADE;
CREATE CLIENT MASTER KEY desc_cmk WITH ( KEY_STORE = gs_ktool , KEY_PATH = "gs_ktool/1" , ALGORITHM = AES_256_CBC);
CREATE COLUMN ENCRYPTION KEY desc_cek WITH VALUES (CLIENT_MASTER_KEY = desc_cmk, ALGORITHM = AEAD_AES_256_CBC_HMAC_SHA256);

create table accounts (
    id serial,
    name varchar(100) not null ENCRYPTED WITH (COLUMN_ENCRYPTION_KEY = desc_cek, ENCRYPTION_TYPE = DETERMINISTIC),
    balance dec(15,2) not null ENCRYPTED WITH (COLUMN_ENCRYPTION_KEY = desc_cek, ENCRYPTION_TYPE = DETERMINISTIC),
    primary key(id)
);

CREATE OR REPLACE FUNCTION f_processed_in_plpgsql(a varchar(100), b dec(15,2))
RETURNS varchar(100) AS $$
declare
c varchar(100);
BEGIN
    SELECT into c name from accounts where name=$1 or balance=$2 LIMIT 1;
    RETURN c;
END; $$
LANGUAGE plpgsql;

\df f_processed_in_plpgsql

DROP FUNCTION f_processed_in_plpgsql();
DROP TABLE accounts;
DROP COLUMN ENCRYPTION KEY desc_cek;
DROP CLIENT MASTER KEY desc_cmk;
\! gs_ktool -d all