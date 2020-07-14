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

#include "app_hooks.h"

/******************************************************************************
* PRIVATE VARIABLES
******************************************************************************/

TS_CALLBACK *TsCallbacks         = 0;
uint32_t     RamStorage_Baudrate = 0xFFFFFFFF;
uint8_t      RamStorage_NodeId   = 0xFF;

/******************************************************************************
* PUBLIC FUNCTIONS
******************************************************************************/

void TS_CallbackInit(TS_CALLBACK *cb)
{
    RamStorage_Baudrate = 0xFFFFFFFF;
    RamStorage_NodeId = 0xFF;

    TsCallbacks = cb;

    cb->NodeFatalError_Called = 0;

    cb->TmrLock_Called = 0;
    cb->TmrUnlock_Called = 0;

    cb->IfReceive_ArgFrm = 0;
    cb->IfReceive_Called = 0;

    cb->PdoTransmit_ArgFrm = 0;
    cb->PdoTransmit_Called = 0;

    cb->PdoReceive_ArgFrm = 0;
    cb->PdoReceive_Called = 0;
    cb->PdoReceive_Return = 0;

    cb->NmtModeChange_ArgNmt = 0;
    cb->NmtModeChange_ArgMode = CO_INVALID;
    cb->NmtModeChange_Called = 0;

    cb->NmtHbConsEvent_ArgNmt = 0;
    cb->NmtHbConsEvent_ArgNodeId = 0;
    cb->NmtHbConsEvent_Called = 0;

    cb->NmtHbConsChange_ArgNmt = 0;
    cb->NmtHbConsChange_ArgNodeId = 0;
    cb->NmtHbConsChange_ArgState = CO_INVALID;
    cb->NmtHbConsChange_Called = 0;

    cb->LssStore_ArgBaudrate = 0;
    cb->LssStore_ArgNodeId = 0;
    cb->LssStore_Called = 0;
    cb->LssStore_Return = CO_ERR_NONE;

    cb->LssLoad_ArgBaudrate = 0;
    cb->LssLoad_ArgNodeId = 0;
    cb->LssLoad_Called = 0;
    cb->LssLoad_Return = CO_ERR_NONE;

    cb->ParaLoad_Called = 0;

    cb->PdoSyncUpdate_ArgPdo = 0;
    cb->PdoSyncUpdate_Called = 0;
}

void TS_CallbackDeInit(void)
{
    RamStorage_Baudrate = 0xFFFFFFFF;
    RamStorage_NodeId = 0xFF;

    TsCallbacks = 0;
}

/******************************************************************************
* PUBLIC CALLBACKS
******************************************************************************/

void CONodeFatalError(void)
{
    if (TsCallbacks != 0) {
        TsCallbacks->NodeFatalError_Called++;
    }
}

void COTmrLock(void)
{
    if (TsCallbacks != 0) {
        TsCallbacks->TmrLock_Called++;
    }
}

void COTmrUnlock(void)
{
    if (TsCallbacks != 0) {
        TsCallbacks->TmrUnlock_Called++;
    }
}

void COIfReceive(CO_IF_FRM *frm)
{
    if (TsCallbacks != 0) {
        TsCallbacks->IfReceive_ArgFrm = frm;
        TsCallbacks->IfReceive_Called++;
    }
}

void COPdoTransmit(CO_IF_FRM *frm)
{
    if (TsCallbacks != 0) {
        TsCallbacks->PdoTransmit_ArgFrm = frm;
        TsCallbacks->PdoTransmit_Called++;
    }
}

int16_t COPdoReceive(CO_IF_FRM *frm)
{
    int16_t result = CO_ERR_NONE;

    if (TsCallbacks != 0) {
        TsCallbacks->PdoReceive_ArgFrm = frm;
        TsCallbacks->PdoReceive_Called++;
        result = TsCallbacks->PdoReceive_Return;
    }

    return (result);
}

void CONmtModeChange(CO_NMT *nmt, CO_MODE mode)
{
    if (TsCallbacks != 0) {
        TsCallbacks->NmtModeChange_ArgNmt  = nmt;
        TsCallbacks->NmtModeChange_ArgMode = mode;
        TsCallbacks->NmtModeChange_Called++;
    }
}

void CONmtHbConsEvent(CO_NMT *nmt, uint8_t nodeId)
{
    if (TsCallbacks != 0) {
        TsCallbacks->NmtHbConsEvent_ArgNmt    = nmt;
        TsCallbacks->NmtHbConsEvent_ArgNodeId = nodeId;
        TsCallbacks->NmtHbConsEvent_Called++;
    }
}

void CONmtHbConsChange(CO_NMT *nmt, uint8_t nodeId, CO_MODE state)
{
    if (TsCallbacks != 0) {
        TsCallbacks->NmtHbConsChange_ArgNmt    = nmt;
        TsCallbacks->NmtHbConsChange_ArgNodeId = nodeId;
        TsCallbacks->NmtHbConsChange_ArgState  = state;
        TsCallbacks->NmtHbConsChange_Called++;
    }
}

int16_t COLssStore(uint32_t baudrate, uint8_t nodeId)
{
    int16_t result = CO_ERR_NONE;

    RamStorage_Baudrate = baudrate;
    RamStorage_NodeId   = nodeId;

    if (TsCallbacks != 0) {
        TsCallbacks->LssStore_ArgBaudrate = baudrate;
        TsCallbacks->LssStore_ArgNodeId   = nodeId;
        TsCallbacks->LssStore_Called++;
        result = TsCallbacks->LssStore_Return;
    }

    return (result);
}

int16_t COLssLoad(uint32_t *baudrate, uint8_t *nodeId)
{
    int16_t result = CO_ERR_NONE;

    if (RamStorage_Baudrate != 0xFFFFFFFFL) {
        *baudrate = RamStorage_Baudrate;
    }
    if (RamStorage_NodeId != 0xFF) {
        *nodeId = RamStorage_NodeId;
    }

    if (TsCallbacks != 0) {
        TsCallbacks->LssLoad_ArgBaudrate = baudrate;
        TsCallbacks->LssLoad_ArgNodeId   = nodeId;
        TsCallbacks->LssLoad_Called++;
        result = TsCallbacks->LssLoad_Return;
    }
    return (result);
}

int16_t COParaLoad(CO_PARA *pg)
{
    int16_t result = CO_ERR_NONE;

    if (TsCallbacks != 0) {
        TsCallbacks->ParaLoad_ArgParaGrp = pg;
        TsCallbacks->ParaLoad_Called++;
        result = TsCallbacks->ParaLoad_Return;
    }

    return (result);
}

int16_t COParaSave(CO_PARA *pg)
{
    int16_t result = CO_ERR_NONE;

    if (TsCallbacks != 0) {
        TsCallbacks->ParaSave_ArgParaGrp = pg;
        TsCallbacks->ParaSave_Called++;
        result = TsCallbacks->ParaSave_Return;
    }

    return (result);
}

int16_t COParaDefault(CO_PARA *pg)
{
    int16_t result = CO_ERR_NONE;

    if (TsCallbacks != 0) {
        TsCallbacks->ParaDefault_ArgParaGrp = pg;
        TsCallbacks->ParaDefault_Called++;
        result = TsCallbacks->ParaDefault_Return;
    }

    return (result);
}

void COPdoSyncUpdate(CO_RPDO *pdo)
{
    if (TsCallbacks != 0) {
        TsCallbacks->PdoSyncUpdate_ArgPdo = pdo;
        TsCallbacks->PdoSyncUpdate_Called++;
    }
}
