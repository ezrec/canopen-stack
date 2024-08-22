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

/******************************************************************************
* INCLUDES
******************************************************************************/

#include "co_core.h"

/******************************************************************************
* MANDATORY CALLBACK FUNCTIONS
******************************************************************************/

WEAK
void CONodeFatalError(CO_NODE *node)
{
    const CO_HOOKS *hooks = &node->Hooks;

    if (hooks->NodeFatalError) {
        hooks->NodeFatalError(node);
    }
}

/******************************************************************************
* OPTIONAL CALLBACK FUNCTIONS
******************************************************************************/

WEAK
void CONmtModeChange(CO_NMT *nmt, CO_MODE mode)
{
    const CO_HOOKS *hooks = &nmt->Node->Hooks;

    if (hooks->NmtModeChange) {
        hooks->NmtModeChange(nmt, mode);
    }
}

WEAK
void CONmtResetRequest(CO_NMT *nmt, CO_NMT_RESET reset)
{
    const CO_HOOKS *hooks = &nmt->Node->Hooks;

    if (hooks->NmtResetRequest) {
        hooks->NmtResetRequest(nmt, reset);
    }
}

WEAK
void CONmtHbConsEvent(CO_NMT *nmt, uint8_t nodeId)
{
    const CO_HOOKS *hooks = &nmt->Node->Hooks;

    if (hooks->NmtHbConsEvent) {
        hooks->NmtHbConsEvent(nmt, nodeId);
    }
}

WEAK
void CONmtHbConsChange(CO_NMT *nmt, uint8_t nodeId, CO_MODE mode)
{
    const CO_HOOKS *hooks = &nmt->Node->Hooks;

    if (hooks->NmtHbConsChange) {
        hooks->NmtHbConsChange(nmt, nodeId, mode);
    }
}

WEAK
CO_ERR COLssLoad(CO_LSS *lss, uint32_t *baudrate, uint8_t *nodeId)
{
    const CO_HOOKS *hooks = &lss->Node->Hooks;
    CO_ERR ret = CO_ERR_NONE;

    if (hooks->LssLoad) {
        ret = hooks->LssLoad(lss, baudrate, nodeId);
    }

    return ret;
}

WEAK
CO_ERR COLssStore(CO_LSS *lss, uint32_t baudrate, uint8_t nodeId)
{
    const CO_HOOKS *hooks = &lss->Node->Hooks;
    CO_ERR ret = CO_ERR_NONE;

    if (hooks->LssStore) {
        ret = hooks->LssStore(lss, baudrate, nodeId);
    }

    return ret;
}

WEAK
void COIfCanReceive(CO_IF *cif, CO_IF_FRM *frm)
{
    const CO_HOOKS *hooks = &cif->Node->Hooks;

    if (hooks->IfCanReceive) {
        hooks->IfCanReceive(cif, frm);
    }
}

WEAK
void COPdoTransmit(CO_TPDO *tpdo, CO_IF_FRM *frm)
{
    const CO_HOOKS *hooks = &tpdo->Node->Hooks;

    if (hooks->PdoTransmit) {
        hooks->PdoTransmit(tpdo, frm);
    }
}

WEAK
int16_t COPdoReceive(CO_RPDO *rpdo, CO_IF_FRM *frm)
{
    const CO_HOOKS *hooks = &rpdo->Node->Hooks;
    int16_t ret = CO_ERR_NONE;

    if (hooks->PdoReceive) {
        ret = hooks->PdoReceive(rpdo, frm);
    }

    return ret;
}

WEAK
void COPdoSyncUpdate(CO_RPDO *rpdo)
{
    const CO_HOOKS *hooks = &rpdo->Node->Hooks;

    if (hooks->PdoSyncUpdate) {
        hooks->PdoSyncUpdate(rpdo);
    }
}

WEAK
int16_t COParaDefault(CO_PARA *pg, CO_NODE *node)
{
    const CO_HOOKS *hooks = &node->Hooks;
    int16_t ret = CO_ERR_NONE;

    if (hooks->ParaDefault) {
        ret = hooks->ParaDefault(pg, node);
    }

    return ret;
}

WEAK
void CORpdoWriteData(CO_RPDO *rpdo, CO_IF_FRM *frm, uint8_t pos, uint8_t size, CO_OBJ *obj)
{
    const CO_HOOKS *hooks = &rpdo->Node->Hooks;

    if (hooks->RPdoWriteData) {
        hooks->RPdoWriteData(rpdo, frm, pos, size, obj);
    }
}

WEAK
void COTpdoReadData(CO_TPDO *tpdo, CO_IF_FRM *frm, uint8_t pos, uint8_t size, CO_OBJ *obj)
{
    const CO_HOOKS *hooks = &tpdo->Node->Hooks;

    if (hooks->TPdoReadData) {
        hooks->TPdoReadData(tpdo, frm, pos, size, obj);
    }
}

WEAK
void COObjUpdated(CO_OBJ *obj, CO_NODE *node)
{
    const CO_HOOKS *hooks = &node->Hooks;

    if (hooks->ObjUpdated) {
        hooks->ObjUpdated(obj, node);
    }
}
