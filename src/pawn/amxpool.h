/*  Simple allocation from a memory pool, with automatic release of
 *  least-recently used blocks (LRU blocks).
 *
 *  Copyright (c) CompuPhase, 2007-2020
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License. You may obtain a copy
 *  of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 *  Version: $Id: amxpool.h 6131 2020-04-29 19:47:15Z thiadmer $
 */
#ifndef AMXPOOL_H_INCLUDED
#define AMXPOOL_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct amxPool {
  void *base;
  unsigned size;
  unsigned short lru;
} amxPool;

void  amx_poolinit(amxPool *pool, void *buffer, unsigned size);
void *amx_poolalloc(amxPool *pool, unsigned size, int index);
void  amx_poolfree(amxPool *pool, void *block);
void *amx_poolfind(amxPool *pool, int index);
int   amx_poolprotect(amxPool *pool, int index);

#if defined(__cplusplus)
}
#endif

#endif /* AMXPOOL_H_INCLUDED */
