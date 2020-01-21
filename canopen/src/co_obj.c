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
* PUBLIC GLOBALS
******************************************************************************/

const CO_OBJ_TYPE COTString = { COTypeStringSize, COTypeStringCtrl, COTypeStringRead, 0 };
const CO_OBJ_TYPE COTDomain = { COTypeDomainSize, COTypeDomainCtrl, COTypeDomainRead, COTypeDomainWrite };

/******************************************************************************
* FUNCTIONS
******************************************************************************/

/*
* see function definition
*/
uint32_t COObjGetSize(CO_OBJ *obj, CO_NODE *node, uint32_t width)
{
    uint32_t     result = 0;
    CO_OBJ_TYPE *type;

    if (obj == 0) {
        return (result);
    }
    type = obj->Type;
    if (type != 0) {
        if (type->Size != 0) {
            result = type->Size(obj, node, width);
        } else {
            result = CO_GET_SIZE(obj->Key);
        }
    } else {
        result = CO_GET_SIZE(obj->Key);
    }

    return (result);
}

/*
* see function definition
*/
int16_t COObjRdValue(CO_OBJ *obj, struct CO_NODE_T *node, void *value, uint8_t width, uint8_t nodeid)
{
    CO_OBJ_TYPE *type;
    int16_t      err = 0;
    uint32_t     val = 0;

    if ((obj == 0) || (value == 0)) {
        return (CO_ERR_BAD_ARG);
    }
    type = obj->Type;
    if (type != 0) {
        err = COObjRdType(obj, node, (void *)&val, CO_LONG, 0);
        if (err != CO_ERR_NONE) {
            return(err);
        }
    } else {
        err = COObjRdDirect(obj, (void *)&val, CO_LONG);
        if (err != CO_ERR_NONE) {
            return(err);
        }
    }
    if (CO_IS_NODEID(obj->Key) != 0) {
        val = (val + nodeid);
    }
    if (width == 1) {
        *((uint8_t *)value) = (uint8_t)val;
    } else if (width == 2) {
        *((uint16_t *)value) = (uint16_t)val;
    } else if (width == 4) {
        *((uint32_t *)value) = (uint32_t)val;
    }

    return(CO_ERR_NONE);
}

/*
* see function definition
*/
int16_t COObjWrValue(CO_OBJ *obj, struct CO_NODE_T *node, void *value, uint8_t width, uint8_t nodeid)
{
    CO_OBJ_TYPE *type;
    uint32_t     val    = 0;
    int16_t      result = CO_ERR_NONE;
    int16_t      status = 0;

    if ((obj == 0) || (value == 0)) {
        return (CO_ERR_BAD_ARG);
    }
    if (width == 1) {
        val = *((uint8_t *)value) & 0xFF;
    } else if (width == 2) {
        val = *((uint16_t *)value) & 0xFFFF;
    } else if (width == 4) {
        val = *((uint32_t *)value);
    } else {
        return (CO_ERR_BAD_ARG);
    }
    if (CO_IS_NODEID(obj->Key) != 0) {
        val = (val - nodeid);
    }

    if (CO_IS_PDOMAP(obj->Key) != 0) {
        status = COObjCmp(obj, (void *)&val);
    }
    type = obj->Type;
    if (type != 0) {
        result = COObjWrType(obj, node, (void *)&val, CO_LONG, 0);
    } else {
        result = COObjWrDirect(obj, (void *)&val, CO_LONG);
    }

    if (status != 0) {
        type = obj->Type;
        if (type == CO_TASYNC) {
            (void)obj->Type->Ctrl(obj, node, CO_TPDO_ASYNC, 0);
        }
    }

    return (result);
}

/*
* see function definition
*/
int16_t COObjRdBufStart(CO_OBJ *obj, struct CO_NODE_T *node, uint8_t *buffer, uint32_t len)
{
    CO_OBJ_TYPE *type;
    int16_t      result = CO_ERR_OBJ_ACC;

    if ((obj == 0) || (buffer == 0)) {
        return (CO_ERR_BAD_ARG);
    }

    type = obj->Type;
    if (type != 0) {
        result = COObjRdType(obj, node, (void *)buffer, len, 0);
    }

    return (result);
}

/*
* see function definition
*/
int16_t COObjRdBufCont(CO_OBJ *obj, struct CO_NODE_T *node, uint8_t *buffer, uint32_t len)
{
    CO_OBJ_TYPE *type;
    int16_t      result = CO_ERR_OBJ_ACC;

    if ((obj == 0) || (buffer == 0)) {
        return (CO_ERR_BAD_ARG);
    }
    type = obj->Type;
    if (type != 0) {
        if (type->Read != 0) {
            result = type->Read(obj, node, buffer, len);
        }
    }

    return (result);
}

/*
* see function definition
*/
int16_t COObjWrBufStart(CO_OBJ *obj, struct CO_NODE_T *node, uint8_t *buffer, uint32_t len)
{
    CO_OBJ_TYPE *type;
    int16_t      result = CO_ERR_OBJ_ACC;

    if ((obj == 0) || (buffer == 0)) {
        return (CO_ERR_BAD_ARG);
    }

    type = obj->Type;
    if (type != 0) {
        result = COObjWrType(obj, node, (void *)buffer, len, 0);
    }

    return (result);
}

/*
* see function definition
*/
int16_t COObjWrBufCont(CO_OBJ *obj, struct CO_NODE_T *node, uint8_t *buffer, uint32_t len)
{
    CO_OBJ_TYPE *type;
    int16_t      result = CO_ERR_OBJ_ACC;

    if ((obj == 0) || (buffer == 0)) {
        return (CO_ERR_BAD_ARG);
    }

    type = obj->Type;
    if (type != 0) {
        if (type->Write != 0) {
            result = type->Write(obj, node, buffer, len);
        }
    }

    return (result);
}

/*
* see function definition
*/
void COObjInit (CO_OBJ *obj)
{
    if (obj == 0) {
        return;
    }
    obj->Key   = 0;
    obj->Type  = 0;
    obj->Data  = 0;
}

/*
* see function definition
*/
int16_t COObjRdDirect(CO_OBJ *obj, void *val, uint32_t len)
{
    uint8_t sz;
    int16_t result = CO_ERR_NONE;

    if ((obj == 0) || (val == 0)) {
        return (CO_ERR_BAD_ARG);
    }

    if (CO_IS_DIRECT(obj->Key) != 0) {
        if (len == CO_BYTE) {
            *((uint8_t *)val) = (uint8_t)((obj->Data)&0xff);
        } else if (len == CO_WORD) {
            *((uint16_t *)val) = (uint16_t)((obj->Data)&0xffff);
        } else if (len == CO_LONG) {
            *((uint32_t *)val) = (uint32_t)(obj->Data);
        } else {
            result = CO_ERR_BAD_ARG;
        }
    } else {
        sz = CO_GET_SIZE(obj->Key);
        if (obj->Data == 0) {
            result = CO_ERR_OBJ_ACC;
        } else if (len == CO_BYTE) {
            if (sz == CO_BYTE) {
                *((uint8_t *)val) = (uint8_t)(*((uint8_t*)(obj->Data))&0xff);
            } else if (sz == CO_WORD) {
                *((uint8_t *)val) = (uint8_t)(*((uint16_t*)(obj->Data))&0xff);
            } else if (sz == CO_LONG) {
                *((uint8_t *)val) = (uint8_t)(*((uint32_t*)(obj->Data))&0xff);
            } else {
                result = CO_ERR_BAD_ARG;
            }
        } else if (len == CO_WORD) {
            if (sz == CO_BYTE) {
                *((uint16_t *)val) = (uint16_t)(*((uint8_t*)(obj->Data))&0xffff);
            } else if (sz == CO_WORD) {
                *((uint16_t *)val) = (uint16_t)(*((uint16_t*)(obj->Data))&0xffff);
            } else if (sz == CO_LONG) {
                *((uint16_t *)val) = (uint16_t)(*((uint32_t*)(obj->Data))&0xffff);
            } else {
                result = CO_ERR_BAD_ARG;
            }
        } else if (len == CO_LONG) {
            if (sz == CO_BYTE) {
                *((uint32_t *)val) = (uint32_t)(*((uint8_t*)(obj->Data)));
            } else if (sz == CO_WORD) {
                *((uint32_t *)val) = (uint32_t)(*((uint16_t*)(obj->Data)));
            } else if (sz == CO_LONG) {
                *((uint32_t *)val) = (uint32_t)(*((uint32_t*)(obj->Data)));
            } else {
                result = CO_ERR_BAD_ARG;
            }
        }
    }

    return (result);
}

/*
* see function definition
*/
int16_t COObjCmp(CO_OBJ *obj, void *val)
{
    uint32_t newval = 0;
    uint32_t oldval = 0;
    int16_t  result = 0;
    int16_t  err;

    newval = *((uint32_t *)val);
    err    = COObjRdDirect(obj, &oldval, CO_LONG);
    if (err != CO_ERR_NONE) {
        return (err);
    }

    if (newval < oldval) {
        result = -1;
    } else if (newval > oldval) {
        result = 1;
    }

    return (result);
}

/*
* see function definition
*/
int16_t COObjRdType(CO_OBJ *obj, struct CO_NODE_T *node, void *dst, uint32_t len, uint32_t off)
{
    CO_OBJ_TYPE *type;
    int16_t      result = CO_ERR_OBJ_ACC;

    type = obj->Type;
    if (type != 0) {
        if (type->Read != 0) {
            if (type->Ctrl != 0) {
                result = type->Ctrl(obj, node, CO_CTRL_SET_OFF, off);
                if (result != CO_ERR_NONE) {
                    return (result);
                }
            }
            result = type->Read(obj, node, dst, len);
        } else {
            result = COObjRdDirect(obj, (void *)dst, len);
        }
    }

    return (result);
}

/*
* see function definition
*/
int16_t COObjWrDirect(CO_OBJ *obj, void *val, uint32_t len)
{
    uint8_t sz;
    int16_t result = CO_ERR_NONE;

    if ((obj == 0) || (val == 0)) {
        return (CO_ERR_BAD_ARG);
    }

    if (CO_IS_DIRECT(obj->Key) != 0) {
        if (len == CO_BYTE) {
            obj->Data = (uint32_t)(*((uint8_t*)val)&0xff);
        } else if (len == CO_WORD) {
            obj->Data = (uint32_t)(*((uint16_t*)val)&0xffff);
        } else if (len == CO_LONG) {
            obj->Data = (uint32_t)(*((uint32_t *)val));
        } else {
            result = CO_ERR_BAD_ARG;
        }
    } else {
        sz = CO_GET_SIZE(obj->Key);
        if (obj->Data == 0) {
            result = CO_ERR_OBJ_ACC;
        } else if (sz == CO_BYTE) {
            if (len == CO_BYTE) {
                *((uint8_t *)(obj->Data)) = (uint8_t)(*((uint8_t*)(val))&0xff);
            } else if (len == CO_WORD) {
                *((uint8_t *)(obj->Data)) = (uint8_t)(*((uint16_t*)(val))&0xff);
            } else if (len == CO_LONG) {
                *((uint8_t *)(obj->Data)) = (uint8_t)(*((uint32_t*)(val))&0xff);
            } else {
                result = CO_ERR_BAD_ARG;
            }
        } else if (sz == CO_WORD) {
            if (len == CO_BYTE) {
                *((uint16_t *)(obj->Data)) = (uint16_t)(*((uint8_t*)(val))&0xffff);
            } else if (len == CO_WORD) {
                *((uint16_t *)(obj->Data)) = (uint16_t)(*((uint16_t*)(val))&0xffff);
            } else if (len == CO_LONG) {
                *((uint16_t *)(obj->Data)) = (uint16_t)(*((uint32_t*)(val))&0xffff);
            } else {
                result = CO_ERR_BAD_ARG;
            }
        } else if (sz == CO_LONG) {
            if (len == CO_BYTE) {
                *((uint32_t *)(obj->Data)) = (uint32_t)(*((uint8_t*)(val)));
            } else if (len == CO_WORD) {
                *((uint32_t *)(obj->Data)) = (uint32_t)(*((uint16_t*)(val)));
            } else if (len == CO_LONG) {
                *((uint32_t *)(obj->Data)) = (uint32_t)(*((uint32_t*)(val)));
            } else {
                result = CO_ERR_BAD_ARG;
            }
        }
    }

    return (result);
}

/*
* see function definition
*/
int16_t COObjWrType(CO_OBJ *obj, CO_NODE *node, void *src, uint32_t len, uint32_t off)
{
    CO_OBJ_TYPE *type;
    int16_t      result = CO_ERR_OBJ_ACC;

    type = obj->Type;
    if (type != 0) {
        if (type->Write != 0) {
            if (type->Ctrl != 0) {
                result = type->Ctrl(obj, node, CO_CTRL_SET_OFF, off);
                if (result != CO_ERR_NONE) {
                    return (result);
                }
            }
            result = type->Write(obj, node, src, len);
        } else {
            result = COObjWrDirect(obj, (void *)src, len);
        }
    }

    return (result);
}

/*
* see function definition
*/
uint32_t COTypeStringSize(CO_OBJ *obj, struct CO_NODE_T *node, uint32_t width)
{
    uint32_t    strlen = 0;
    CO_OBJ_STR *str;
    uint8_t    *ptr;

    (void)width;
    str = (CO_OBJ_STR *)(obj->Data);
    ptr = str->Start;
    while (*ptr != '\0') {
        strlen++;
        ptr++;
    }

    return (strlen);
}

/*
* see function definition
*/
int16_t COTypeStringCtrl(CO_OBJ *obj, struct CO_NODE_T *node, uint16_t func, uint32_t para)
{
    int16_t     result = CO_ERR_TYPE_CTRL;
    CO_OBJ_STR *str;

    str = (CO_OBJ_STR *)(obj->Data);
    if (func == CO_CTRL_SET_OFF) {
        str->Offset = para;
        result = CO_ERR_NONE;
    }

    return (result);
}

/*
* see function definition
*/
int16_t COTypeStringRead(CO_OBJ *obj, struct CO_NODE_T *node, void *buf, uint32_t len)
{
    uint32_t    offset;
    int16_t     result = CO_ERR_NONE;
    CO_OBJ_STR *str;
    uint8_t    *ptr;
    uint8_t    *dst;

    str    = (CO_OBJ_STR *)(obj->Data);
    offset = str->Offset;
    ptr    = (uint8_t *)(str->Start) + offset;
    dst    = (uint8_t *)buf;
    while ((*ptr != 0) && (len  > 0)) {
        *dst = *ptr;
        offset++;
        ptr++;
        dst++;
        len--;
    }
    str->Offset = offset;

    return (result);
}

/*
* see function definition
*/
uint32_t COTypeDomainSize(CO_OBJ *obj, struct CO_NODE_T *node, uint32_t width)
{
    CO_OBJ_DOM *dom;
    uint32_t    result = 0;

    if (obj->Data == 0) {
        return (result);
    }
    dom = (CO_OBJ_DOM *)(obj->Data);
    if ((width > 0) && (width < dom->Size)) {
        result = width;
    } else {
        result = dom->Size;
    }

    return (result);
}

/*
* see function definition
*/
int16_t COTypeDomainCtrl(CO_OBJ *obj, struct CO_NODE_T *node, uint16_t func, uint32_t para)
{
    CO_OBJ_DOM *dom;
    int16_t     result = CO_ERR_TYPE_CTRL;

    dom = (CO_OBJ_DOM *)(obj->Data);
    if (func == CO_CTRL_SET_OFF) {
        dom->Offset = para;
        result = CO_ERR_NONE;
    }

    return (result);
}

/*
* see function definition
*/
int16_t COTypeDomainRead(CO_OBJ *obj, struct CO_NODE_T *node, void *buf, uint32_t len)
{
    CO_OBJ_DOM *dom;
    int16_t     result = CO_ERR_NONE;
    uint8_t    *src;
    uint8_t    *dst;
    uint32_t    num;

    dom = (CO_OBJ_DOM *)(obj->Data);
    src = (uint8_t *)(dom->Start + dom->Offset);
    num = dom->Size - dom->Offset;
    dst = (uint8_t *)buf;
    while ((len > 0) && (num > 0)) {
        *dst = *src;
        src++;
        dst++;
        len--;
        dom->Offset++;
        num--;
    }

    return (result);
}

/*
* see function definition
*/
int16_t COTypeDomainWrite(CO_OBJ *obj, struct CO_NODE_T *node, void *buf, uint32_t len)
{
    CO_OBJ_DOM *dom;
    int16_t     result = CO_ERR_NONE;
    uint8_t    *src;
    uint8_t    *dst;
    uint32_t    num;

    dom = (CO_OBJ_DOM *)(obj->Data);
    dst = (uint8_t *)(dom->Start + dom->Offset);
    num = dom->Size - dom->Offset;
    src = (uint8_t *)buf;
    while ((len > 0) && (num > 0)) {
        *dst = *src;
        src++;
        dst++;
        len--;
        dom->Offset++;
        num--;
    }

    return (result);
}
