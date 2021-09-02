/* -------------------------------------------------------------------------
 *
 * define.cpp
 *	  Support routines for various kinds of object creation.
 *
 *
 * Portions Copyright (c) 2020 Huawei Technologies Co.,Ltd.
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/gausskernel/optimizer/commands/define.cpp
 *
 * DESCRIPTION
 *	  The "DefineFoo" routines take the parse tree and pick out the
 *	  appropriate arguments/flags, passing the results to the
 *	  corresponding "FooDefine" routines (in src/catalog) that do
 *	  the actual catalog-munging.  These routines also verify permission
 *	  of the user to execute the command.
 *
 * NOTES
 *	  These things must be defined and committed in the following order:
 *		"create function":
 *				input/output, recv/send procedures
 *		"create type":
 *				type
 *		"create operator":
 *				operators
 *
 *
 * -------------------------------------------------------------------------
 */
#ifndef FRONTEND_PARSER
#include "postgres.h"
#include "knl/knl_variable.h"
#else
#include "postgres_fe.h"
#endif

#include <ctype.h>
#include <math.h>
#ifdef FRONTEND_PARSER
#include "nodes/parsenodes_common.h"
#include "commands/defrem.h"
#include "nodes/makefuncs.h"
#include "parser/scansup.h"
#else
#include "catalog/namespace.h"
#include "commands/defrem.h"
#include "nodes/makefuncs.h"
#include "parser/parse_type.h"
#include "parser/scansup.h"
#include "utils/int8.h"
#endif

#ifndef FRONTEND_PARSER
const int INTEGER_SIZE = 64;

/*
 * Extract a string value (otherwise uninterpreted) from a DefElem.
 */
char* defGetString(DefElem* def)
{
    if (def->arg == NULL)
        ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("%s requires a parameter", def->defname)));
    switch (nodeTag(def->arg)) {
        case T_Integer: {
            char* str = (char*)palloc(INTEGER_SIZE);

            errno_t rc = snprintf_s(str, INTEGER_SIZE, INTEGER_SIZE - 1, "%ld", (long)intVal(def->arg));
            securec_check_ss(rc, "\0", "\0");
            return str;
        }
        case T_Float:

            /*
             * T_Float values are kept in string form, so this type cheat
             * works (and doesn't risk losing precision)
             */
            return strVal(def->arg);
        case T_String:
            return strVal(def->arg);
        case T_TypeName:
            return TypeNameToString((TypeName*)def->arg);
        case T_List:
            return NameListToString((List*)def->arg);
        case T_A_Star:
            return pstrdup("*");
        default:
            ereport(ERROR,
                (errcode(ERRCODE_UNRECOGNIZED_NODE_TYPE),
                    errmsg("unrecognized node type: %d", (int)nodeTag(def->arg))));
    }
    return NULL; /* keep compiler quiet */
}

/*
 * Extract a numeric value (actually double) from a DefElem.
 */
double defGetNumeric(DefElem* def)
{
    if (def->arg == NULL)
        ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("%s requires a numeric value", def->defname)));
    switch (nodeTag(def->arg)) {
        case T_Integer:
            return (double)intVal(def->arg);
        case T_Float:
            return floatVal(def->arg);
        default:
            ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("%s requires a numeric value", def->defname)));
    }
    return 0; /* keep compiler quiet */
}

/*
 * Extract a boolean value from a DefElem.
 */
bool defGetBoolean(DefElem* def)
{
    /*
     * If no parameter given, assume "true" is meant.
     */
    if (def->arg == NULL)
        return true;

    /*
     * Allow 0, 1, "true", "false", "on", "off"
     */
    switch (nodeTag(def->arg)) {
        case T_Integer:
            switch (intVal(def->arg)) {
                case 0:
                    return false;
                case 1:
                    return true;
                default:
                    /* otherwise, error out below */
                    break;
            }
            break;
        default: {
            char* sval = defGetString(def);

            /*
             * The set of strings accepted here should match up with the
             * grammar's opt_boolean production.
             */
            if (pg_strcasecmp(sval, "true") == 0)
                return true;
            if (pg_strcasecmp(sval, "false") == 0)
                return false;
            if (pg_strcasecmp(sval, "on") == 0)
                return true;
            if (pg_strcasecmp(sval, "off") == 0)
                return false;
        } break;
    }
    ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("%s requires a Boolean value", def->defname)));
    return false; /* keep compiler quiet */
}

/*
 * Extract an int64 value from a DefElem.
 */
int64 defGetInt64(DefElem* def)
{
    if (def->arg == NULL)
        ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("%s requires a numeric value", def->defname)));
    switch (nodeTag(def->arg)) {
        case T_Integer:
            return (int64)intVal(def->arg);
        case T_Float:

            /*
             * Values too large for int4 will be represented as Float
             * constants by the lexer.	Accept these if they are valid int8
             * strings.
             */
            return DatumGetInt64(DirectFunctionCall1(int8in, CStringGetDatum(strVal(def->arg))));
        default:
            ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("%s requires a numeric value", def->defname)));
    }
    return 0; /* keep compiler quiet */
}

/*
 * Extract a possibly-qualified name (as a List of Strings) from a DefElem.
 */
List* defGetQualifiedName(DefElem* def)
{
    if (def->arg == NULL)
        ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("%s requires a parameter", def->defname)));
    switch (nodeTag(def->arg)) {
        case T_TypeName:
            return ((TypeName*)def->arg)->names;
        case T_List:
            return (List*)def->arg;
        case T_String:
            /* Allow quoted name for backwards compatibility */
            return list_make1(def->arg);
        default:
            ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("argument of %s must be a name", def->defname)));
    }
    return NIL; /* keep compiler quiet */
}

/*
 * Extract a TypeName from a DefElem.
 *
 * Note: we do not accept a List arg here, because the parser will only
 * return a bare List when the name looks like an operator name.
 */
TypeName* defGetTypeName(DefElem* def)
{
    if (def->arg == NULL)
        ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("%s requires a parameter", def->defname)));
    switch (nodeTag(def->arg)) {
        case T_TypeName:
            return (TypeName*)def->arg;
        case T_String:
            /* Allow quoted typename for backwards compatibility */
            return makeTypeNameFromNameList(list_make1(def->arg));
        default:
            ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("argument of %s must be a type name", def->defname)));
    }
    return NULL; /* keep compiler quiet */
}

/*
 * Extract a type length indicator (either absolute bytes, or
 * -1 for "variable") from a DefElem.
 */
int defGetTypeLength(DefElem* def)
{
    if (def->arg == NULL)
        ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("%s requires a parameter", def->defname)));
    switch (nodeTag(def->arg)) {
        case T_Integer:
            return intVal(def->arg);
        case T_Float:
            ereport(ERROR, (errcode(ERRCODE_SYNTAX_ERROR), errmsg("%s requires an integer value", def->defname)));
            break;
        case T_String:
            if (pg_strcasecmp(strVal(def->arg), "variable") == 0)
                return -1; /* variable length */
            break;
        case T_TypeName:
            /* cope if grammar chooses to believe "variable" is a typename */
            if (pg_strcasecmp(TypeNameToString((TypeName*)def->arg), "variable") == 0)
                return -1; /* variable length */
            break;
        case T_List:
            /* must be an operator name */
            break;
        default:
            ereport(ERROR,
                (errcode(ERRCODE_UNRECOGNIZED_NODE_TYPE),
                    errmsg("unrecognized node type: %d", (int)nodeTag(def->arg))));
    }
    ereport(ERROR,
        (errcode(ERRCODE_SYNTAX_ERROR), errmsg("invalid argument for %s: \"%s\"", def->defname, defGetString(def))));
    return 0; /* keep compiler quiet */
}
#endif /* !FRONTEND_PARSER */

/*
 * Create a DefElem setting "oids" to the specified value.
 */
DefElem* defWithOids(bool value)
{
    return makeDefElem("oids", (Node*)makeInteger(value));
}

/*
 * Set value with specific option, or add a new def value
 */
List* defSetOption(List* options, const char* name, Node* value)
{
    ListCell* lc = NULL;
    foreach (lc, options) {
        DefElem* elem = (DefElem*)lfirst(lc);

        if (strncmp(elem->defname, name, strlen(name)) == 0) {
            elem->arg = value;
            break;
        }
    }
#ifndef FRONTEND_PARSER
    if (lc == NULL)
        options = lappend(options, makeDefElem(pstrdup(name), value));
#else
    if (lc == NULL)
        options = lappend(options, makeDefElem(strdup(name), value));
#endif 

    return options;
}