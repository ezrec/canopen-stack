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

/* TODO: rename the include file name to match the naming convention:
 *   co_can_<device>.h
 */
#include "drv_can_dummy.h"

/******************************************************************************
* PRIVATE DEFINES
******************************************************************************/

/* TODO: place here your CAN controller register definitions */

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void    DrvCanInit   (CO_IF *cif);
static void    DrvCanEnable (CO_IF *cif, uint32_t baudrate);
static int16_t DrvCanSend   (CO_IF *cif, CO_IF_FRM *frm);
static int16_t DrvCanRead   (CO_IF *cif, CO_IF_FRM *frm);
static void    DrvCanReset  (CO_IF *cif);
static void    DrvCanClose  (CO_IF *cif);

/******************************************************************************
* PUBLIC VARIABLE
******************************************************************************/

/* TODO: rename the variable to match the naming convention:
 *   <device>CanDriver
 */
const CO_IF_CAN_DRV DummyCanDriver = {
    DrvCanInit,
    DrvCanEnable,
    DrvCanRead,
    DrvCanSend,
    DrvCanReset,
    DrvCanClose
};

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void DrvCanInit(CO_IF *cif)
{
    /* TODO: initialize the CAN controller (don't enable communication) */
}

static void DrvCanEnable(CO_IF *cif, uint32_t baudrate)
{
    (void)baudrate;

    /* TODO: set the given baudrate to the CAN controller */
}

static int16_t DrvCanSend(CO_IF *cif, CO_IF_FRM *frm)
{
    (void)frm;

    /* TODO: wait for free CAN message slot and send the given CAN frame */
    return (0u);
}

static int16_t DrvCanRead (CO_IF *cif, CO_IF_FRM *frm)
{
    (void)frm;

    /* TODO: wait for a CAN frame and read CAN frame from the CAN controller */
    return (0u);
}

static void DrvCanReset(CO_IF *cif)
{
    /* TODO: reset CAN controller while keeping baudrate */
}

static void DrvCanClose(CO_IF *cif)
{
    /* TODO: remove CAN controller from CAN network */
}
