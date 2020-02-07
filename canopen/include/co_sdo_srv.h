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

#ifndef CO_SDO_SRV_H_
#define CO_SDO_SRV_H_

/******************************************************************************
* INCLUDES
******************************************************************************/

#include "co_types.h"
#include "co_cfg.h"
#include "co_if.h"

/******************************************************************************
* PUBLIC DEFINES
******************************************************************************/

#define CO_SDO_ERR_TBIT         0x05030000    /*!< Toggle bit not alternated              */
#define CO_SDO_ERR_CMD          0x05040001    /*!< SDO command specifier invalid/unknown  */
#define CO_SDO_ERR_BLK_SIZE     0x05040002    /*!< Invalid block size                     */
#define CO_SDO_ERR_SEQ_NUM      0x05040003    /*!< Invalid Sequence number                */
#define CO_SDO_ERR_RD           0x06010001    /*!< Attempt to read a write only object    */
#define CO_SDO_ERR_WR           0x06010002    /*!< Attempt to write a read only object    */
#define CO_SDO_ERR_OBJ          0x06020000    /*!< Object doesn't exist in dictionary      */
#define CO_SDO_ERR_LEN_HIGH     0x06070012    /*!< Length of parameter too high           */
#define CO_SDO_ERR_LEN_SMALL    0x06070013    /*!< Length of parameter too small          */
#define CO_SDO_ERR_SUB          0x06090011    /*!< Subindex doesn't exist in dictionary    */
#define CO_SDO_ERR_RANGE        0x06090030    /*!< Value range of parameter exceeded      */
#define CO_SDO_ERR_TOS          0x08000020    /*!< Data can't be transfered or stored     */
#define CO_SDO_ERR_TOS_STATE    0x08000022    /*!< Data can't be transfered or stored,    */
                                              /*   because of the present device state    */
#define CO_SDO_ERR_OBJ_MAP      0x06040041    /*!< Object cannot be mapped to the PDO     */
#define CO_SDO_ERR_OBJ_MAP_N    0x06040042    /*!< Number and length exceed PDO           */
#define CO_SDO_ERR_PARA_INCOMP  0x06040043    /*!< parameter incompatibility reason       */

#define CO_SDO_RD               1             /*!< Object read access                     */
#define CO_SDO_WR               2             /*!< Object write access                    */

#define CO_SDO_ID_OFF      ((uint32_t)1<<31)  /*!< Disabled SDO server / COBID            */

#define CO_SDO_BUF_SEG     127
#define CO_SDO_BUF_BYTE    (CO_SDO_BUF_SEG*7) /*!< transfer buffer size in byte           */

#define CO_TSDOID  ((CO_OBJ_TYPE *)&COTSdoId) /*!< Object Type Dynamic SDO Identifier     */

/******************************************************************************
* PUBLIC CONSTANTS
******************************************************************************/

/*! \brief OBJECT TYPE SDO IDENTIFIER
*
*    This object type specializes the general handling of objects for the
*    object dictionary entries holding a SDO identifier. These entries are
*    designed to provide the feature of changing a SDO identifier.
*/
extern const CO_OBJ_TYPE COTSdoId;
    
/******************************************************************************
* PUBLIC TYPES
******************************************************************************/

/*! \brief SDO TRANSFER BUFFER
*
*    This structure holds the data, which are needed for the SDO transfer
*    buffer.
*/
typedef struct CO_SDO_BUF_T {
    uint32_t  Num;               /*!< Number of bytes in transfer buffer     */
    uint8_t  *Start;             /*!< Pointer to start of transfer buffer    */
    uint8_t  *Cur;               /*!< Pointer to next free buffer location   */

} CO_SDO_BUF;

/*! \brief SDO SEGMENTED TRANSFER
*
*    This structure holds the data, which are needed for the segmented
*    SDO transfer.
*/
typedef struct CO_SDO_SEG_T {
    uint32_t  Size;              /*!< Size of object entry                   */
    uint32_t  Num;               /*!< Number of transfered bytes             */
    uint8_t   TBit;              /*!< Segment toggle bit                     */

} CO_SDO_SEG;

/*! \brief SDO BLOCK TRANSFER STATES
*
*    This enumeration holds the possible SDO server block transfer states.
*/
typedef enum CO_SDO_BLK_STATE_T {
    BLK_IDLE,                    /*!< no block transfer ongoing              */
    BLK_DOWNLOAD,                /*!< block download active                  */
    BLK_UPLOAD,                  /*!< block upload active                    */
    BLK_REPEAT,                  /*!< block upload repeat request active     */
    BLK_DNWAIT                   /*!< block download wait for next block/end */

} CO_SDO_BLK_STATE;

/*! \brief SDO BLOCK TRANSFER
*
*    This structure holds the data, which are needed for the SDO block
*    transfer.
*/
typedef struct CO_SDO_BLK_T {
    enum CO_SDO_BLK_STATE_T State;      /*!< block transfer state            */
    uint32_t                Size;       /*!< Size of object entry            */
    uint32_t                Len;        /*!< remaining block length in byte  */
    uint8_t                 SegNum;     /*!< number of segments in block     */
    uint8_t                 SegCnt;     /*!< current segment number          */
    uint8_t                 LastValid;  /*!< valid bytes in last segment     */

} CO_SDO_BLK;

/*! \brief SDO SERVER
*
*    This structure holds all data, which are needed for managing a
*    single SDO server.
*/
typedef struct CO_SDO_T {
    struct CO_NODE_T   *Node;    /*!< Link to node info structure            */
    struct CO_OBJ_T    *Obj;     /*!< Addressed Object Entry Reference       */
    CO_IF_FRM          *Frm;     /*!< SDO request/response CAN frame         */
    uint32_t            RxId;    /*!< SDO request CAN identifier             */
    uint32_t            TxId;    /*!< SDO response CAN identifier            */
    uint16_t            Idx;     /*!< Extracted Multiplexer Index            */
    uint8_t             Sub;     /*!< Extracted Multiplexer Subindex         */
    struct CO_SDO_BUF_T Buf;     /*!< Transfer buffer management structure   */
    struct CO_SDO_SEG_T Seg;     /*!< Segmented transfer control structure   */
    struct CO_SDO_BLK_T Blk;     /*!< Block transfer control structure       */

} CO_SDO;

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

/*! \brief  INIT SDO SERVER
*
*    This function reads the content of the object dictionary with
*    index 1200+[n] subindex 1 (for RX) and subindex 2 (for TX), where n
*    is a counter from 0 to the maximal number of supported SDO servers
*    (CO_SDO_N).
*
* \param srv
*    Ptr to root element of SDO server array
*
* \param node
*    Ptr to parent CANopen node
*
* \internal
*/
void COSdoInit(CO_SDO *srv, struct CO_NODE_T *node);

/*! \brief  RESET SDO SERVER
*
*    This function resets all internal data of the given SDO server 'num'.
*
* \param srv
*    Ptr to root element of SDO server array
*
* \param num
*    Number of SDO server
*
* \param node
*    Ptr to parent CANopen node
*
* \internal
*/
void COSdoReset(CO_SDO *srv, uint8_t num, struct CO_NODE_T *node);

/*! \brief  ENABLE SDO SERVER
*
*    This function reads the content of the object dictionary for a
*    given SDO server number 'num' from the with index 1200+[num] subindex 1
*    (for RX) and subindex 2 (for TX). The identifiers for request and
*    response will be set acc. the CANopen rules. If everything is ok,
*    the SDO server is enabled after this function call.
*
* \param srv
*    Ptr to root element of SDO server array
*
* \param num
*    Number of SDO server
*
* \internal
*/
void COSdoEnable(CO_SDO *srv, uint8_t num);

/*! \brief  CHECK FOR SDO FRAME
*
*    This function checks the given frame to be a SDO request. If the frame
*    is identified to be a SDO request, the identifier in the frame will be
*    modified to be the corresponding SDO response.
*
* \param srv
*    Ptr to root element of SDO server array
*
* \param frm
*    Frame, received from CAN bus
*
* \retval  >0    pointer to addressed SDO server
* \retval  =0    not a SDO request or an error is detected.
*
* \internal
*/
CO_SDO *COSdoCheck(CO_SDO *srv, CO_IF_FRM *frm);

/*! \brief  GENERATE SDO RESPONSE
*
*    This function interprets the data byte #0 of the SDO request and
*    organizes the generation of the corresponding response.
*
* \param srv
*    Ptr to root element of SDO server array
*
* \retval  =0    successful SDO response
* \retval  <0    an error is detected
*
* \internal
*/
int16_t COSdoResponse(CO_SDO *srv);

/*! \brief  GET ADDRESSED OBJECT
*
*    This function looks for the addressed object in the object dictionary
*    and generates several abort messages on bad addresses. If everything
*    is fine, this function stores the corresponding object entry reference
*    in the SDO server object and returns 0.
*
* \param srv
*    Ptr to root element of SDO server array
*
* \param mode
*    Access mode (\ref CO_SDO_RD or \ref CO_SDO_WR)
*
* \retval  =0    successful operation
* \retval  <0    abort
*
* \internal
*/
int16_t COSdoGetObject(CO_SDO *srv, uint16_t mode);

/*! \brief  GET SIZE OF OBJECT
*
*    This function checks the given size acc. the addressed object size in
*    the object dictionary and generates several abort messages on bad size
*    informations. If everything is fine, this function returns the size of
*    the object in byte.
*
* \param srv
*    Pointer to SDO server object
*
* \param width
*    Parameter size in byte (or 0 if unknown)
*
* \retval  =0    Abort for known width
* \retval  >0    Size of Object
*
* \internal
*/
uint32_t COSdoGetSize(CO_SDO *srv, uint32_t width);

/*! \brief  EXPEDITED UPLOAD PROTOCOL
*
*    This function generates the response for a SDO expedited upload request.
*    The function checks the object size for matching into a simple expedited
*    transfer. If possible this transfer is used. The segmented transfer
*    (if support available) is initiated in any other cases.
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    successful operation
* \retval  <0    abort
*
* \internal
*/
int16_t COSdoUploadExpedited(CO_SDO *srv);

/*! \brief  EXPEDITED DOWNLOAD PROTOCOL
*
*    This function generates the response for a SDO expedited download
*    request.
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    successful operation
* \retval  <0    abort
*
* \internal
*/
int16_t COSdoDownloadExpedited(CO_SDO *srv);

/*! \brief  ABORT PROTOCOL
*
*    This function generates the response for a SDO abort.
*
* \param srv
*    Pointer to SDO server object
*
* \param err
*    The error code to be inserted in the data bytes #4..#7
*
* \internal
*/
void COSdoAbort(CO_SDO *srv, uint32_t err);

/*! \brief  INIT SEGMENTED UPLOAD
*
*    This function generates the response for 'Initiate SDO Upload Protocol'.
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     2     | x | x | x | x | x |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition := "command & 0xE0 must be 0x40"*
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \param size
*    Object size in byte
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*/
int16_t COSdoInitUploadSegmented(struct CO_SDO_T *srv, uint32_t width);

/*! \brief  SEGMENTED UPLOAD
*
*    This function generates the response for 'Upload SDO Segment Protocol'.
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     3     | t |       0       |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command & 0xEF must be 0x60"
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*/
int16_t COSdoUploadSegmented(struct CO_SDO_T *srv);

/*! \brief  INIT SEGMENTED DOWNLOAD
*
*    This function generates the response for 'Initiate SDO Download
*    protocol'.
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     1     | 0 |   n   | 0 | s |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command & 0xF2 must be 0x20"
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*/
int16_t COSdoInitDownloadSegmented(struct CO_SDO_T *srv);

/*! \brief  SEGMENTED DOWNLOAD
*
*    This function generates the response for 'Download SDO Segment
*    Protocol'.
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     0     | t |     n     | c |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command & 0xE0 must be 0x00"
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*/
int16_t COSdoDownloadSegmented(struct CO_SDO_T *srv);

/*! \brief  INIT BLOCK DOWNLOAD
*
*    This function generates the response for 'Initiate SDO Block Download
*    Protocol'.
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     6     |   0   | cc| s | 0 |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command & 0xF9 must be 0xC0"
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*/
int16_t COSdoInitDownloadBlock(struct CO_SDO_T *srv);

/*! \brief  END BLOCK DOWNLOAD
*
*    This function generates the response for 'End SDO Block Download
*    Protocol'.
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     6     |     n     | 0 | 1 |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command & 0xE3 must be 0xC1"
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*/
int16_t COSdoEndDownloadBlock(struct CO_SDO_T *srv);

/*! \brief  BLOCK DOWNLOAD
*
*    This function generates the response for 'Download SDO Block Segment
*    Protocol'.
*
*    Entry condition for this function is activation of the block download
*    state. The SDO request command byte contains the state after this
*    CAN frame:
* \code
*    +---+---+---+---+---+---+---+---+
*    | c |      0 < seqno < 128      |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => c=0: more segments to be downloaded
*       C=1: enter END SDO block download phase
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*/
int16_t COSdoDownloadBlock(struct CO_SDO_T *srv);

/*! \brief  INIT BLOCK UPLOAD
*
*    This function generates the response for 'Initiate SDO Block Upload
*    Protocol', sub-command 'initate upload request'
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     5     | 0 | cc| 0 |   0   |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command & 0xE7 must be 0xA0"
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*
* \todo     Implement the PST to dynamically switch to segmented or
*           expedited transfer if number of segments is below the
*           given threshold.
*/
int16_t COSdoInitUploadBlock(struct CO_SDO_T *srv);

/*! \brief  BLOCK UPLOAD
*
*    This function generates the response for 'Initiate SDO Block Upload
*    Protocol', sub-command 'start upload'.
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition, or the already started block upload
*    state:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     5     |     0     |   3   |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command must be 0xA3"
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*/
int16_t COSdoUploadBlock(struct CO_SDO_T *srv);

/*! \brief  CONFIRM BLOCK UPLOAD
*
*    This function generates the response for 'Upload SDO Block Segment
*    Protocol'.
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     5     | x | x | x |   2   |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command & 0xE3 must be 0xA2"
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*/
int16_t COSdoAckUploadBlock(struct CO_SDO_T *srv);

/*! \brief  END BLOCK UPLOAD
*
*    This function generates the response for 'End SDO Block Upload Protocol'.
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     5     |     0     |   1   |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command must be 0xA1"
* \endcode
*
* \param srv
*    Pointer to SDO server object
*
* \retval  =0    on success
* \retval  <0    on SDO abort
*
* \internal
*/
int16_t COSdoEndUploadBlock(struct CO_SDO_T *srv);

/*! \brief  SDO ABORT REQUEST
*
*    This function is called on receiving an SDO abort request. The
*    requested SDO transfer will be cancled.
*
* \param srv
*    Pointer to SDO server object
*
* \internal
*/
void COSdoAbortReq(CO_SDO *srv);

/*! \brief  WRITE SDO IDENTIFIER
*
*    This write function is responsible for the object write procedure for
*    a write access to a SDO CAN identifier.
*
* \param obj
*    Ptr to addressed object entry
*
* \param node
*    reference to parent node
*
* \param buf
*    Ptr to data buffer
*
* \param size
*    Size of given data in buffer
*
* \retval   =CO_ERR_NONE    Successfully operation
* \retval  !=CO_ERR_NONE    An error is detected
*
* \internal
*/
int16_t COTypeSdoIdWrite(CO_OBJ* obj, struct CO_NODE_T *node, void *buf, uint32_t size);

#endif  /* #ifndef CO_SDO_H_ */
