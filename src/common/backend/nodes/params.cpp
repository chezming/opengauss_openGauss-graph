/* -------------------------------------------------------------------------
 *
 * params.cpp
 *	  Support for finding the values associated with Param nodes.
 *
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/common/backend/nodes/params.cpp
 *
 * -------------------------------------------------------------------------
 */

#include "postgres.h"
#include "knl/knl_variable.h"

#include "nodes/params.h"
#include "utils/datum.h"
#include "utils/lsyscache.h"

/*
 * Copy a ParamListInfo structure.
 *
 * The result is allocated in CurrentMemoryContext.
 *
 * Note: the intent of this function is to make a static, self-contained
 * set of parameter values.  If dynamic parameter hooks are present, we
 * intentionally do not copy them into the result.	Rather, we forcibly
 * instantiate all available parameter values and copy the datum values.
 */
ParamListInfo copyParamList(ParamListInfo from)
{
    ParamListInfo retval;
    Size size;
    int i;

    if (from == NULL || from->numParams <= 0) {
        return NULL;
    }

    size = offsetof(ParamListInfoData, params) + from->numParams * sizeof(ParamExternData);

    retval = (ParamListInfo)palloc(size);
    retval->paramFetch = NULL;
    retval->paramFetchArg = NULL;
    retval->parserSetup = NULL;
    retval->parserSetupArg = NULL;
    retval->params_need_process = false;
    retval->numParams = from->numParams;
    
    for (i = 0; i < from->numParams; i++) {
        ParamExternData* oprm = &from->params[i];
        ParamExternData* nprm = &retval->params[i];
        int16 typLen;
        bool typByVal = false;

        /* give hook a chance in case parameter is dynamic */
        if (!OidIsValid(oprm->ptype) && from->paramFetch != NULL) {
            (*from->paramFetch)(from, i + 1);
        }

        /* flat-copy the parameter info */
        *nprm = *oprm;

        /* need datumCopy in case it's a pass-by-reference datatype */
        if (nprm->isnull || !OidIsValid(nprm->ptype)) {
            continue;
        }
        get_typlenbyval(nprm->ptype, &typLen, &typByVal);
        nprm->value = datumCopy(nprm->value, typByVal, typLen);
        nprm->tabInfo = NULL;
        if (oprm->tabInfo != NULL) {
            nprm->tabInfo = (TableOfInfo*)palloc0(sizeof(TableOfInfo));
            nprm->tabInfo->tableOfIndexType = oprm->tabInfo->tableOfIndexType;
            nprm->tabInfo->tableOfIndex = copyTableOfIndex(oprm->tabInfo->tableOfIndex);
            nprm->tabInfo->isnestedtable = oprm->tabInfo->isnestedtable;
            nprm->tabInfo->tableOfLayers = oprm->tabInfo->tableOfLayers;
        }
    }

    return retval;
}
