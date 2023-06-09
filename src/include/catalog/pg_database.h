/* -------------------------------------------------------------------------
 *
 * pg_database.h
 *      definition of the system "database" relation (pg_database)
 *      along with the relation's initial contents.
 *
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/pg_database.h
 *
 * NOTES
 *      the genbki.pl script reads this file and generates .bki
 *      information from the DATA() statements.
 *
 * -------------------------------------------------------------------------
 */
#ifndef PG_DATABASE_H
#define PG_DATABASE_H

#include "catalog/genbki.h"

/* ----------------
 *        pg_database definition.  cpp turns this into
 *        typedef struct FormData_pg_database
 * ----------------
 */
#define DatabaseRelationId    1262
#define DatabaseRelation_Rowtype_Id  1248

CATALOG(pg_database,1262) BKI_SHARED_RELATION BKI_ROWTYPE_OID(1248) BKI_SCHEMA_MACRO
{
    NameData    datname;              /* database name */
    Oid         datdba;               /* owner of database */
    int4        encoding;             /* character encoding */
    NameData    datcollate;           /* LC_COLLATE setting */
    NameData    datctype;             /* LC_CTYPE setting */
    bool        datistemplate;        /* allowed as CREATE DATABASE template? */
    bool        datallowconn;         /* new connections allowed? */
    int4        datconnlimit;         /* max connections allowed (-1=no limit) */
    Oid         datlastsysoid;        /* highest OID to consider a system OID */
    ShortTransactionId datfrozenxid;  /* all Xids < this are frozen in this DB */
    Oid         dattablespace;        /* default table space for this DB */
    NameData    datcompatibility;

#ifdef CATALOG_VARLEN            /* variable-length fields start here */
    aclitem        datacl[1];        /* access permissions */
#endif
    TransactionId datfrozenxid64; /* all Xids < this are frozen in this DB */
    TransactionId datminmxid;	/* all multixacts in the DB are >= this */
} FormData_pg_database;

/* Size of fixed part of pg_database tuples, not counting var-length fields */
#define DATABASE_TUPLE_SIZE \
     (offsetof(FormData_pg_database, datcompatibility) + sizeof(NameData))
     
/* ----------------
 *        Form_pg_database corresponds to a pointer to a tuple with
 *        the format of pg_database relation.
 * ----------------
 */
typedef FormData_pg_database *Form_pg_database;

/* ----------------
 *        compiler constants for pg_database
 * ----------------
 */
#define Natts_pg_database				15
#define Anum_pg_database_datname		1
#define Anum_pg_database_datdba			2
#define Anum_pg_database_encoding		3
#define Anum_pg_database_datcollate		4
#define Anum_pg_database_datctype		5
#define Anum_pg_database_datistemplate	6
#define Anum_pg_database_datallowconn	7
#define Anum_pg_database_datconnlimit	8
#define Anum_pg_database_datlastsysoid	9
#define Anum_pg_database_datfrozenxid	10
#define Anum_pg_database_dattablespace	11
#define Anum_pg_database_compatibility	12
#define Anum_pg_database_datacl			13
#define Anum_pg_database_datfrozenxid64	14
#define Anum_pg_database_datminmxid		15

DATA(insert OID = 1 (  template1 PGUID ENCODING "LC_COLLATE" "LC_CTYPE" t t -1 0 0 1663 "DB_COMPATIBILITY" _null_ 3 1));
SHDESCR("unmodifiable empty database");
#define TemplateDbOid            1

#define DEFAULT_DATABASE  "postgres"

#endif   /* PG_DATABASE_H */

