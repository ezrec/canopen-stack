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
* GLOBAL CONSTANTS
******************************************************************************/

const CO_OBJ_TYPE COTSdoId = { 0, 0, 0, COTypeSdoIdWrite };

/******************************************************************************
* FUNCTIONS
******************************************************************************/

/*
* see function definition
*/
void COSdoInit(CO_SDO *srv, CO_NODE *node)
{
    uint8_t n;

    for (n=0; n < CO_SDOS_N; n++) {
        COSdoReset (srv, n, node);
        COSdoEnable(srv, n);
    }
}

/*
* see function definition
*/
void COSdoReset(CO_SDO *srv, uint8_t num, CO_NODE *node)
{
    CO_SDO   *srvnum;
    uint32_t  offset;

    if (srv == 0) {
        return;
    }
    if (num >= CO_SDOS_N) {
        return;
    }

    srvnum               = &srv[num];
    srvnum->Node         = node;
    srvnum->RxId         = CO_SDO_ID_OFF;
    srvnum->TxId         = CO_SDO_ID_OFF;
    srvnum->Frm          = 0;
    srvnum->Obj          = 0;
    offset               = num * CO_SDO_BUF_BYTE;
    srvnum->Buf.Start    = &node->SdoBuf[offset];
    srvnum->Buf.Cur      = &node->SdoBuf[offset];
    srvnum->Buf.Num      = 0;
    srvnum->Seg.TBit     = 0;
    srvnum->Seg.Num      = 0;
    srvnum->Seg.Size     = 0;
    srvnum->Blk.State    = BLK_IDLE;
}

/*
* see function definition
*/
void COSdoEnable(CO_SDO *srv, uint8_t num)
{
    uint32_t rxId;
    uint32_t txId;
    CO_NODE *node;
    CO_SDO  *srvnum;
    int16_t  err;

    if (num >= CO_SDOS_N) {
        return;
    }
    srvnum       = &srv[num];
    srvnum->RxId = CO_SDO_ID_OFF;
    srvnum->TxId = CO_SDO_ID_OFF;

    node = srv->Node;
    err  = CODictRdLong(&node->Dict, CO_DEV(0x1200 + num, 1), &rxId);
    if (err != CO_ERR_NONE) {
        return;
    }
    err = CODictRdLong(&node->Dict, CO_DEV(0x1200 + num, 2), &txId);
    if (err != CO_ERR_NONE) {
        return;
    }

    if (((rxId & CO_SDO_ID_OFF) == 0) &&
        ((txId & CO_SDO_ID_OFF) == 0) ) {
        srvnum->RxId = rxId;
        srvnum->TxId = txId;
    }
}

/*
* see function definition
*/
CO_SDO *COSdoCheck(CO_SDO *srv, CO_IF_FRM *frm)
{
    CO_SDO  *result = 0;
    uint8_t  n;

    if (frm != 0) {
        n = 0;
        while ((n < CO_SDOS_N) && (result == 0)) {
            if (CO_GET_ID(frm) == srv[n].RxId) {
                CO_SET_ID(frm, srv[n].TxId);
                srv[n].Frm = frm;
                if (srv[n].Obj == 0) {
                    srv[n].Idx = CO_GET_WORD(frm, 1);
                    srv[n].Sub = CO_GET_BYTE(frm, 3);
                }
                result = &srv[n];
            }
            n++;
        }
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoResponse(CO_SDO *srv)
{
    int16_t result = -1;
    uint8_t cmd;

    cmd = CO_GET_BYTE(srv->Frm, 0);

    /* client abort */
    if (cmd == 0x80) {
        COSdoAbortReq(srv);
        return (-2);
    }

    /* active block transfer */
    if (srv->Blk.State == BLK_DOWNLOAD) {
        result = COSdoDownloadBlock(srv);
        return (result);
    } else if (srv->Blk.State == BLK_DNWAIT) {
        if ((cmd & 0xE3) == 0xC1) {
            result = COSdoEndDownloadBlock(srv);
        } else {
            srv->Blk.State   = BLK_DOWNLOAD;
            result = COSdoDownloadBlock(srv);
        }
        return (result);
    } else if (srv->Blk.State == BLK_UPLOAD) {
        if (cmd == 0xA1) {
            result = COSdoEndUploadBlock(srv);
        } else if ((cmd & 0xE3) == 0xA2) {
            result = COSdoAckUploadBlock(srv);
        } else {
            COSdoAbort(srv, CO_SDO_ERR_CMD);
        }
        return (result);
    }

    /* expedited transfer */
    if ((cmd & 0xF2) == 0x22) {
        result = COSdoGetObject(srv, CO_SDO_WR);
        if (result == 0) {
            result = COSdoDownloadExpedited(srv);
        }
    } else if (cmd == 0x40) {
        result = COSdoGetObject(srv, CO_SDO_RD);
        if (result == 0) {
            result = COSdoUploadExpedited(srv);
        }

    /* segmented transfer */
    } else if ((cmd & 0xF2) == 0x20) {
        result = COSdoGetObject(srv, CO_SDO_WR);
        if (result == 0) {
            result = COSdoInitDownloadSegmented(srv);
        }
    } else if ((cmd & 0xE0) == 0x00) {
        result = COSdoDownloadSegmented(srv);
    } else if ((cmd & 0xEF) == 0x60) {
        result = COSdoUploadSegmented(srv);
    
    /* block transfer */
    } else if ((cmd & 0xF9) == 0xC0) {
        result = COSdoGetObject(srv, CO_SDO_WR);
        if (result == 0) {
            result = COSdoInitDownloadBlock(srv);
        }
    } else if ((cmd & 0xE3) == 0xA0) {
        result = COSdoInitUploadBlock(srv);
    } else if (cmd == 0xA3) {
        result = COSdoUploadBlock(srv);

    /* invalid or unknown command */
    } else {
        COSdoAbort(srv, CO_SDO_ERR_CMD);
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoGetObject(CO_SDO *srv, uint16_t mode)
{
    CO_OBJ  *obj;
    uint32_t key;
    int16_t  result = -1;

    key = CO_DEV(srv->Idx, srv->Sub);
    obj = CODictFind(&srv->Node->Dict, key);
    if (obj != 0) {
        if (mode == CO_SDO_RD) {
            if (CO_IS_READ(obj->Key) != 0) {
                srv->Obj = obj;
                result   = 0;
            } else {
                COSdoAbort(srv, CO_SDO_ERR_RD);
            }
        } else {
            if (CO_IS_WRITE(obj->Key) != 0) {
                srv->Obj = obj;
                result   = 0;
            } else {
                COSdoAbort(srv, CO_SDO_ERR_WR);
            }
        }
    } else {
        if (srv->Sub == 0) {
            COSdoAbort(srv, CO_SDO_ERR_OBJ);
        } else {
            key = CO_DEV(srv->Idx, 0);
            obj = CODictFind(&srv->Node->Dict, key);
            if (obj != 0) {
                COSdoAbort(srv, CO_SDO_ERR_SUB);
            } else {
                COSdoAbort(srv, CO_SDO_ERR_OBJ);
            }
        }
    }

    return (result);
}

/*
* see function definition
*/
uint32_t COSdoGetSize(CO_SDO *srv, uint32_t width)
{
    uint32_t result = 0;
    uint32_t size;

    size = COObjGetSize(srv->Obj, srv->Node, width);
    if (size == 0) {
        COSdoAbort(srv, CO_SDO_ERR_TOS);
        return (result);
    }

    if (width != 0) {
        if (size == width) {
            result = size;
        } else {
            if (size < width) {
                COSdoAbort(srv, CO_SDO_ERR_LEN_HIGH);
            } else {
                COSdoAbort(srv, CO_SDO_ERR_LEN_SMALL);
            }
        }
    } else {
        result = size;
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoUploadExpedited(CO_SDO *srv)
{
    uint32_t size;
    uint32_t data   = 0;
    int16_t  result = -1;
    int16_t  num;
    uint8_t  cmd;
    uint8_t  nodeid;

    size = COSdoGetSize(srv, 0);
    if (size == 0) {
        return (result);
    } else if ((size > 0) && (size <= 4)) {
        nodeid = srv->Node->NodeId;
        num    = COObjRdValue(srv->Obj, srv->Node, (void *)&data, CO_LONG, nodeid);
        if (num != CO_ERR_NONE) {
            COSdoAbort(srv, CO_SDO_ERR_TOS);
            return (result);
        } else {
            cmd = 0x43 | (((4-size) & 0x3) << 2);
            CO_SET_BYTE(srv->Frm,  cmd, 0);
            CO_SET_LONG(srv->Frm, data, 4);
            srv->Obj = 0;
            result   = 0;
        }
    }

    if (size > 4) {
        result = COSdoInitUploadSegmented(srv, size);
    }

    if (result < 0) {
        COSdoAbort(srv, CO_SDO_ERR_CMD);
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoDownloadExpedited(CO_SDO *srv)
{
    uint32_t size;
    uint32_t data;
    uint32_t width  =  0;
    int16_t  result = -1;
    int16_t  num;
    uint8_t  cmd;
    uint8_t  nodeid;

    cmd = CO_GET_BYTE(srv->Frm, 0);
    if ((cmd & 0x01) == 1) {
        width = 4 - ((cmd >> 2) & 0x03);
    }
    size = COSdoGetSize(srv, width);
    if ((size > 0) && (size <= 4)) {
        nodeid = srv->Node->NodeId;
        data   = CO_GET_LONG(srv->Frm, 4);
        num    = COObjWrValue(srv->Obj, srv->Node, (void*)&data, CO_LONG, nodeid);
        if (num != CO_ERR_NONE) {
            if (num ==  CO_ERR_OBJ_RANGE) {
                COSdoAbort(srv, CO_SDO_ERR_RANGE);
            } else if (num == CO_ERR_OBJ_MAP_TYPE) {
                COSdoAbort(srv, CO_SDO_ERR_OBJ_MAP);
            } else if (num == CO_ERR_OBJ_ACC) {
                COSdoAbort(srv, CO_SDO_ERR_TOS);
            } else if (num == CO_ERR_OBJ_MAP_LEN) {
                COSdoAbort(srv, CO_SDO_ERR_OBJ_MAP_N);
            } else if (num == CO_ERR_OBJ_INCOMPATIBLE) {
                COSdoAbort(srv, CO_SDO_ERR_PARA_INCOMP);
            } else {
                COSdoAbort(srv, CO_SDO_ERR_TOS);
            }
        } else {
            CO_SET_BYTE(srv->Frm, 0x60, 0);
            CO_SET_LONG(srv->Frm,    0, 4);
            srv->Obj = 0;
            result   = 0;
        }
    }

    return (result);
}

/*
* see function definition
*/
void COSdoAbort(CO_SDO *srv, uint32_t err)
{
    CO_SET_BYTE(srv->Frm,     0x80, 0);
    CO_SET_WORD(srv->Frm, srv->Idx, 1);
    CO_SET_BYTE(srv->Frm, srv->Sub, 3);
    CO_SET_LONG(srv->Frm,      err, 4);

    srv->Obj = 0;
}

/*
* see function definition
*/
int16_t COSdoInitUploadSegmented(CO_SDO *srv, uint32_t size)
{
    int16_t result;
    uint8_t cmd;

    cmd = 0x41;
    CO_SET_BYTE(srv->Frm, cmd, 0);
    CO_SET_LONG(srv->Frm, size, 4);

    srv->Seg.Size = size;
    srv->Seg.TBit = 0;
    srv->Seg.Num  = 0;
    srv->Buf.Cur  = srv->Buf.Start;
    srv->Buf.Num  = 0;

    result = COObjRdBufStart(srv->Obj, srv->Node, srv->Buf.Cur, 0);
    if (result != CO_ERR_NONE) {
        srv->Node->Error = CO_ERR_SDO_READ;
        result           = CO_ERR_NONE;
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoInitDownloadSegmented(CO_SDO *srv)
{
    uint32_t size;
    uint32_t width  =  0;
    int16_t  result = -1;
    uint8_t  cmd;

    cmd = CO_GET_BYTE(srv->Frm, 0);
    if ((cmd & 0x01) == 1) {
        width = CO_GET_LONG(srv->Frm, 4);
    }
    size = COSdoGetSize(srv, width);
    if (size > 0) {
        CO_SET_BYTE(srv->Frm, 0x60, 0);
        CO_SET_LONG(srv->Frm, 0, 4);

        srv->Seg.Size = size;
        srv->Seg.TBit = 0;
        srv->Seg.Num  = 0;
        srv->Buf.Cur  = srv->Buf.Start;
        srv->Buf.Num  = 0;
        
        result = COObjWrBufStart(srv->Obj, srv->Node, srv->Buf.Cur, 0);
        if (result != CO_ERR_NONE) {
            srv->Node->Error = CO_ERR_SDO_WRITE;
            result           = CO_ERR_NONE;
        }
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoDownloadSegmented(CO_SDO *srv)
{
    uint32_t num;
    uint32_t len;
    int16_t  result = 0;
    uint8_t  n;
    uint8_t  cmd;
    uint8_t  bid;

    cmd = CO_GET_BYTE(srv->Frm, 0);
    if ((cmd >> 4) != srv->Seg.TBit) {
        COSdoAbort(srv, CO_SDO_ERR_TBIT);
        return (-1);
    }

    n = ((cmd >> 1) & 0x07);
    if (n == 0) {
        num = srv->Seg.Size - srv->Seg.Num;
        if (num > 7) {
            num = 7;
        }
    } else {
        num = 7 - n;
    }

    bid = 1;
    while (num > 0) {
        *(srv->Buf.Cur) = CO_GET_BYTE(srv->Frm, bid);
        srv->Buf.Num++;
        srv->Buf.Cur++;
        bid++;
        num--;
    }
    srv->Seg.Num += srv->Buf.Num;

    if ((cmd & 0x01) == 0x01) {
        len    = (uint32_t)srv->Buf.Num;
        result = COObjWrBufCont(srv->Obj, srv->Node, srv->Buf.Start, len);
        if (result != CO_ERR_NONE) {
            COSdoAbort(srv, CO_SDO_ERR_TOS);
            result = -1;
        }
        srv->Seg.Size = 0;
        srv->Seg.Num  = 0;
        srv->Obj      = 0;
    } else {
        len    = (uint32_t)srv->Buf.Num;
        result = COObjWrBufCont(srv->Obj, srv->Node, srv->Buf.Start, len);
        if (result != CO_ERR_NONE) {
            COSdoAbort(srv, CO_SDO_ERR_TOS);
            result = -1;
        }
        srv->Buf.Cur  = srv->Buf.Start;
        srv->Buf.Num  = 0;
    }

    cmd = (uint8_t)((1 << 5) | (srv->Seg.TBit << 4));
    if (result == 0) {
        CO_SET_BYTE(srv->Frm, cmd, 0);
        CO_SET_BYTE(srv->Frm, 0, 1);
        CO_SET_WORD(srv->Frm, 0, 2);
        CO_SET_LONG(srv->Frm, 0, 4);
    }

    srv->Seg.TBit ^= 0x01;

    return (result);
}

/*
* see function definition
*/
int16_t COSdoUploadSegmented(CO_SDO *srv)
{
    uint32_t width;
    int16_t  result = 0;
    uint8_t  cmd;
    uint8_t  c_bit  = 0;
    uint8_t  i;

    if (srv->Obj == 0) {
        COSdoAbort(srv, CO_SDO_ERR_CMD);
        return (-1);
    }

    cmd = CO_GET_BYTE(srv->Frm, 0);
    if (((cmd >> 4) & 0x01) != srv->Seg.TBit) {
        COSdoAbort(srv, CO_SDO_ERR_TBIT);
        return (-1);
    }

    width = srv->Seg.Size - srv->Seg.Num;
    if (width > 7) {
        width = 7;
    } else {
        c_bit = 1;
    }

    if (c_bit == 1) {
        result = COObjRdBufCont(srv->Obj, srv->Node, srv->Buf.Start, width);
    } else {
        result = COObjRdBufCont(srv->Obj, srv->Node, srv->Buf.Start, width);
    }
    if (result != CO_ERR_NONE) {
        COSdoAbort(srv, CO_SDO_ERR_TOS);
        return (-1);
    }

    srv->Buf.Cur = srv->Buf.Start;
    for (i = 0; i < (uint8_t)width; i++) {
        CO_SET_BYTE(srv->Frm, *(srv->Buf.Cur), 1 + i);
        srv->Buf.Cur++;
    }
    for (i = (uint8_t)(width + 1); i <= 7; i++) {
        CO_SET_BYTE(srv->Frm, 0, 1 + i);
    }

    cmd = (uint8_t)0x00 |
          (uint8_t)(srv->Seg.TBit << 4) |
          (uint8_t)(((7 - width) << 1) & 0x0E) |
          (uint8_t)c_bit;
    CO_SET_BYTE(srv->Frm, cmd, 0);

    if (c_bit == 1) {
        srv->Seg.Size  = 0;
        srv->Seg.Num   = 0;
        srv->Seg.TBit  = 0;
        srv->Obj       = 0;
    } else {
        srv->Seg.Num  += width;
        srv->Seg.TBit ^= 0x01;
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoInitDownloadBlock(CO_SDO *srv)
{
    uint32_t size;
    uint32_t width  =  0;
    int16_t  result = -1;
    uint8_t  cmd;

    cmd = CO_GET_BYTE(srv->Frm, 0);
    if ((cmd & 0x02) != 0) {
        width = CO_GET_LONG(srv->Frm, 4);
    }
    size = COSdoGetSize(srv, width);
    if (size == 0) {
        COSdoAbort(srv, CO_SDO_ERR_TOS);
        return (result);
    }
    if (width <= size) {
        result = COObjWrBufStart(srv->Obj, srv->Node, srv->Buf.Cur, 0);
        if (result != CO_ERR_NONE) {
            srv->Node->Error = CO_ERR_SDO_WRITE;
            result           = CO_ERR_NONE;
        }

        srv->Blk.SegCnt = 0;
        srv->Blk.State  = BLK_DOWNLOAD;
        srv->Blk.SegNum = CO_SDO_BUF_SEG;

        CO_SET_BYTE(srv->Frm, 0xA0, 0);
        CO_SET_LONG(srv->Frm, (uint32_t)CO_SDO_BUF_SEG, 4);
    } else {
        COSdoAbort(srv, CO_SDO_ERR_LEN_HIGH);
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoEndDownloadBlock(CO_SDO *srv)
{
    uint32_t len;
    int16_t  result = -1;
    uint8_t  cmd;
    uint8_t  n;

    cmd = CO_GET_BYTE(srv->Frm, 0);
    if ((cmd & 0x01) != 0) {
        n      = (cmd & 0x1C) >> 2;
        len    = ((uint32_t)srv->Buf.Num - n);
        result = COObjWrBufCont(srv->Obj, srv->Node, srv->Buf.Start, len);
        if (result != CO_ERR_NONE) {
            srv->Node->Error = CO_ERR_SDO_WRITE;
            result = -1;
        }
        CO_SET_BYTE(srv->Frm, 0xA1, 0);
        CO_SET_WORD(srv->Frm, 0, 1);
        CO_SET_BYTE(srv->Frm, 0, 3);
        CO_SET_LONG(srv->Frm, 0, 4);

        srv->Blk.State = BLK_IDLE;
        srv->Buf.Cur   = srv->Buf.Start;
        srv->Buf.Num   = 0;
        srv->Obj       = 0;
        result         = 0;
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoDownloadBlock(CO_SDO *srv)
{
    uint32_t len;
    int16_t  result = -2;
    int16_t  err;
    uint8_t  cmd;
    uint8_t  i;

    cmd = CO_GET_BYTE(srv->Frm, 0);
    if ((cmd & 0x7F) == (srv->Blk.SegCnt + 1)) {
        for (i = 0; i < 7; i++) {
            *(srv->Buf.Cur) = CO_GET_BYTE(srv->Frm, 1 + i);
            srv->Buf.Cur++;
            srv->Buf.Num++;
        }
        srv->Blk.SegCnt++;
        if ((srv->Blk.SegCnt == CO_SDO_BUF_SEG) ||
             ((cmd & 0x80)   != 0             )) {

            CO_SET_BYTE(srv->Frm, 0xA2, 0);
            CO_SET_BYTE(srv->Frm, srv->Blk.SegCnt, 1);
            CO_SET_BYTE(srv->Frm, CO_SDO_BUF_SEG, 2);
            for (i = 3; i <= 7; i++) {
                CO_SET_BYTE(srv->Frm, 0, i);
            }
            srv->Blk.SegCnt  = 0;
            srv->Blk.State   = BLK_DNWAIT;
            result           = 0;
        }

        if (result == 0) {
            if ((cmd & 0x80) == 0) {
                len = (uint32_t)srv->Buf.Num;
                err = COObjWrBufCont(srv->Obj, srv->Node, srv->Buf.Start, len);
                if (err != CO_ERR_NONE) {
                    srv->Node->Error = CO_ERR_SDO_WRITE;
                }
                srv->Buf.Cur = srv->Buf.Start;
                srv->Buf.Num = 0;
            }
        }
    } else {
        if ((srv->Blk.SegCnt & 0x80) == 0) {
            srv->Blk.SegCnt |= 0x80;
        }

        if (((cmd & 0x7F) == CO_SDO_BUF_SEG) ||
            ((cmd & 0x80) != 0             )) {

            CO_SET_BYTE(srv->Frm, 0xA2, 0);
            CO_SET_BYTE(srv->Frm, srv->Blk.SegCnt & 0x7F, 1);
            CO_SET_BYTE(srv->Frm, CO_SDO_BUF_SEG, 2);
            CO_SET_BYTE(srv->Frm, 0, 3);
            CO_SET_LONG(srv->Frm, 0, 4);

            srv->Blk.SegCnt = 0;
            srv->Buf.Cur    = srv->Buf.Start;
            srv->Buf.Num    = 0;
            result          = 0;
        }
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoInitUploadBlock(CO_SDO *srv)
{
    uint32_t size;
    int16_t  result = 0;
    int16_t  err;
    uint8_t  cmd;

    err = COSdoGetObject(srv, CO_SDO_RD);
    if (err < 0) {
        return (-1);
    }

    srv->Blk.Size = COSdoGetSize(srv, 0);
    if (srv->Blk.Size == 0) {
        return (-1);
    }
    srv->Blk.SegNum = CO_GET_BYTE(srv->Frm, 4);

    if ((srv->Blk.SegNum < 0x01) ||
        (srv->Blk.SegNum > 0x7F)) {
        COSdoAbort(srv, CO_SDO_ERR_BLK_SIZE);
        COSdoAbortReq(srv);
        return (-1);
    } else {
        if (srv->Blk.SegNum > CO_SDO_BUF_SEG) {
            srv->Blk.SegNum = CO_SDO_BUF_SEG;
        }
    }

    size  = srv->Blk.Size;
    cmd   = 0xC2;

    srv->Blk.LastValid = 0xFF;
    srv->Blk.Len       = srv->Blk.Size;
    
    err = COObjRdBufStart(srv->Obj, srv->Node, srv->Buf.Cur, 0);
    if (err != CO_ERR_NONE) {
        srv->Node->Error = CO_ERR_SDO_READ;
    }
    CO_SET_BYTE(srv->Frm, cmd, 0);
    CO_SET_LONG(srv->Frm, size, 4);

    return (result);
}

/*
* see function definition
*/
int16_t COSdoUploadBlock(CO_SDO *srv)
{
    uint32_t size;
    uint32_t num;
    int16_t  result   = -2;
    int16_t  err;
    uint8_t  finished =  0;
    uint8_t  seg;
    uint8_t  len;
    uint8_t  i;

    srv->Buf.Cur = srv->Buf.Start;
    srv->Buf.Num = 7 * srv->Blk.SegNum;
    if (srv->Blk.State == BLK_REPEAT) {
        srv->Blk.Len += srv->Buf.Num;
    } else {
        if (srv->Blk.Size > srv->Buf.Num) {
            num = (uint32_t)srv->Buf.Num;
            err = COObjRdBufCont(srv->Obj, srv->Node, srv->Buf.Start, num);
            if (err != CO_ERR_NONE) {
                srv->Node->Error = CO_ERR_SDO_READ;
            }
            srv->Blk.Size -= srv->Buf.Num;
        } else {
            num = (uint32_t)srv->Buf.Num;
            srv->Buf.Num   = srv->Blk.Size;
            err = COObjRdBufCont(srv->Obj, srv->Node, srv->Buf.Start, num);
            if (err != CO_ERR_NONE) {
                srv->Node->Error = CO_ERR_SDO_READ;
            }
            srv->Blk.Size = 0;
        }
    }
    srv->Blk.State  = BLK_UPLOAD;
    srv->Blk.SegCnt = 1;
    while ((srv->Blk.SegCnt <= srv->Blk.SegNum) && (finished == 0)) {
        seg  = srv->Blk.SegCnt;
        size = srv->Blk.Len;
        if (size > 7) {
            len           = 7;
            srv->Blk.Len -= 7;
            if (srv->Blk.SegCnt < srv->Blk.SegNum) {
                srv->Blk.SegCnt++;
            } else {
                finished = 1;
            }
        } else {
            len          = (uint8_t)size;
            srv->Blk.Len = 0;
            finished     = 1;
        }
        if (finished == 1) {
            if (srv->Blk.Len == 0) {
                seg |= 0x80;
            }
            srv->Blk.LastValid  = len;
        }
        CO_SET_BYTE(srv->Frm, seg, 0);
        for (i = 0; i < len; i++) {
            CO_SET_BYTE(srv->Frm, *(srv->Buf.Cur), 1+i);
            srv->Buf.Cur++;
            srv->Buf.Num--;
        }
        for (i = (uint8_t)len; i < 7; i++) {
            CO_SET_BYTE(srv->Frm, 0, 1 + i);
        }
        (void)COIfSend(&srv->Node->If, srv->Frm);
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoAckUploadBlock(CO_SDO *srv)
{
    int16_t result = -2;
    uint8_t cmd;
    uint8_t seq;
    uint8_t val;
    uint8_t i;

    seq = CO_GET_BYTE(srv->Frm, 1);
    if (seq > 0x7F) {
        COSdoAbort(srv, CO_SDO_ERR_SEQ_NUM);
        COSdoAbortReq(srv);
        return (-1);
    } else if (seq != srv->Blk.SegCnt) {
        srv->Blk.State  = BLK_REPEAT;
        result          = COSdoUploadBlock(srv);
    } else if (srv->Blk.Len == 0) {
        if (srv->Blk.LastValid <= 7) {
            val = (uint8_t)srv->Blk.LastValid;
            cmd = (uint8_t)0xC0 |
                  (uint8_t)((7u - val) << 2) |
                  (uint8_t)0x01;
            CO_SET_BYTE(srv->Frm, cmd, 0);
            for (i = 1; i <= 7; i++) {
                CO_SET_BYTE(srv->Frm, 0, i);
            }
            result = 0;
        }
    } else {
        srv->Blk.SegNum = CO_GET_BYTE(srv->Frm, 2);
        if ((srv->Blk.SegNum < 0x01) ||
            (srv->Blk.SegNum > 0x7F)) {
            COSdoAbort(srv, CO_SDO_ERR_BLK_SIZE);
            COSdoAbortReq(srv);
            return (-1);
        } else {
            if (srv->Blk.SegNum > CO_SDO_BUF_SEG) {
                srv->Blk.SegNum = CO_SDO_BUF_SEG;
            }
        }
        result = COSdoUploadBlock(srv);
    }

    return (result);
}

/*
* see function definition
*/
int16_t COSdoEndUploadBlock(CO_SDO *srv)
{
    int16_t result = -2;

    srv->Blk.State = BLK_IDLE;
    srv->Obj       = 0;

    return (result);
}

/*
* see function definition
*/
void COSdoAbortReq(CO_SDO *srv)
{
    srv->Obj       =  0;
    srv->Idx       =  0;
    srv->Sub       =  0;
    srv->Buf.Cur   =  srv->Buf.Start;
    srv->Buf.Num   =  0;
    srv->Blk.State =  BLK_IDLE;
    srv->Seg.Num   =  0;
    srv->Seg.Size  =  0;
    srv->Seg.TBit  =  0;
}

/*
* see function definition
*/
int16_t COTypeSdoIdWrite(CO_OBJ* obj, struct CO_NODE_T *node, void *buf, uint32_t size)
{
    uint32_t  newval;
    uint32_t  curval;
    int16_t   err = CO_ERR_NONE;
    uint8_t   num;

    if ((obj == 0) || (buf == 0) || (size != CO_LONG)) {
        return (CO_ERR_BAD_ARG);
    }
    newval = *(uint32_t *)buf;
    (void)COObjRdDirect(obj, &curval, CO_LONG);
    num    = CO_GET_IDX(obj->Key) & 0x7F;

    if ((curval & CO_SDO_ID_OFF) == 0) {
        if ((newval & CO_SDO_ID_OFF) != 0) {
            err = COObjWrDirect(obj, &newval, CO_LONG);
            if (err == CO_ERR_NONE) {
                COSdoReset(node->Sdo, num, node);
            }
        } else {
            return (CO_ERR_OBJ_RANGE);
        }
    } else {
        err = COObjWrDirect(obj, &newval, CO_LONG);
    }
    if (err == CO_ERR_NONE) {
        COSdoEnable(node->Sdo, num);
    }

    return (err);
}
