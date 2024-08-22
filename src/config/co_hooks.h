/******************************************************************************
   Copyright 2020 Embedded Office GmbH & Co. KG

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
******************************************************************************/

#ifndef CO_HOOKS_H_
#define CO_HOOKS_H_

#ifdef __cplusplus               /* for compatibility with C++ environments  */
extern "C" {
#endif

/******************************************************************************
* INCLUDES
******************************************************************************/

#include <stdint.h>

#include <co_nmt.h>

struct CO_NODE_T;
struct CO_LSS_T;
struct CO_IF_T;
struct CO_TPDO_T;
struct CO_RPDO_T;
struct CO_PARA_T;
struct CO_IF_FRM_T;

typedef struct CO_HOOKS_T {
    void (*NodeFatalError)(struct CO_NODE_T *node);
    void (*NmtModeChange)(struct CO_NMT_T *nmt, enum CO_MODE_T mode);
    void (*NmtResetRequest)(struct CO_NMT_T *nmt, enum CO_NMT_RESET_T reset);
    void (*NmtHbConsEvent)(struct CO_NMT_T *nmt, uint8_t nodeId);
    void (*NmtHbConsChange)(struct CO_NMT_T *nmt, uint8_t nodeId, enum CO_MODE_T mode);
    CO_ERR (*LssLoad)(struct CO_LSS_T *lss, uint32_t *baudrate, uint8_t *nodeId);
    CO_ERR (*LssStore)(struct CO_LSS_T *lss, uint32_t baudrate, uint8_t nodeId);
    void (*IfCanReceive)(struct CO_IF_T *cif, struct CO_IF_FRM_T *frm);
    void (*PdoTransmit)(struct CO_TPDO_T *tpdo, struct CO_IF_FRM_T *frm);
    int16_t (*PdoReceive)(struct CO_RPDO_T *rpdo, struct CO_IF_FRM_T *frm);
    void (*PdoSyncUpdate)(struct CO_RPDO_T *pdo);
    int16_t (*ParaDefault)(struct CO_PARA_T *pg, struct CO_NODE_T *node);
    void (*RPdoWriteData)(struct CO_RPDO_T *rpdo, struct CO_IF_FRM_T *frm, uint8_t pos, uint8_t size, CO_OBJ *obj);
    void (*TPdoReadData)(struct CO_TPDO_T *tdpo, struct CO_IF_FRM_T *frm, uint8_t pos, uint8_t size, CO_OBJ *obj);
    void (*ObjUpdated)(CO_OBJ *obj, struct CO_NODE_T *node);
} CO_HOOKS;

#ifdef __cplusplus               /* for compatibility with C++ environments  */
}
#endif

#endif  /* #ifndef CO_HOOKS_H_ */
