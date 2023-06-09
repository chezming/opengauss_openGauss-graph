/* -------------------------------------------------------------------------
 *
 * nodeNestloop.h
 *
 *
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeNestloop.h
 *
 * -------------------------------------------------------------------------
 */
#ifndef NODENESTLOOP_H
#define NODENESTLOOP_H

#include "nodes/execnodes.h"

extern NestLoopState* ExecInitNestLoop(NestLoop* node, EState* estate, int eflags);
extern TupleTableSlot* ExecNestLoop(NestLoopState* node);
extern void ExecEndNestLoop(NestLoopState* node);
extern void ExecReScanNestLoop(NestLoopState* node);

#ifdef GS_GRAPH
extern void ExecPrevNestLoopContext(NestLoopState *node);
extern void ExecNextNestLoopContext(NestLoopState *node);
#endif
#endif /* NODENESTLOOP_H */
