/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "ble_gatts_app.h"
#include "ble_serialization.h"
#include "ble_gatts.h"
#include "app_util.h"


uint32_t ble_gatts_include_add_req_enc(uint16_t         service_handle,
                                       uint16_t         inc_srvc_handle,
                                       uint16_t * const p_include_handle,
                                       uint8_t * const  p_buf,
                                       uint32_t * const p_buf_len)
{

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);
    SER_ASSERT_LENGTH_LEQ(6, *p_buf_len);

    uint32_t index   = 0;
    uint32_t buf_len = *p_buf_len;
    uint32_t err_code;
    uint8_t  opCode = SD_BLE_GATTS_INCLUDE_ADD;
    uint8_t  presenceFlag;

    err_code = uint8_t_enc(&opCode, p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    err_code = uint16_t_enc(&service_handle, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    err_code = uint16_t_enc(&inc_srvc_handle, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    presenceFlag = (p_include_handle != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;
    err_code     = uint8_t_enc(&presenceFlag, p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    *p_buf_len = index;
    return err_code;

}


uint32_t ble_gatts_include_add_rsp_dec(uint8_t const * const p_buf,
                                       uint32_t              packet_len,
                                       uint16_t * const      p_include_handle,
                                       uint32_t * const      p_result_code)
{
    uint32_t index = 0;
    uint32_t err_code;

    err_code = ser_ble_cmd_rsp_result_code_dec(p_buf, &index, packet_len,
                                               SD_BLE_GATTS_INCLUDE_ADD,
                                               p_result_code);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, packet_len);
        return NRF_SUCCESS;
    }
    SER_ASSERT_NOT_NULL(p_include_handle);
    SER_ASSERT_LENGTH_LEQ(index + 2, packet_len);
    err_code = uint16_t_dec(p_buf, packet_len, &index, p_include_handle);
    SER_ASSERT_LENGTH_EQ(index, packet_len);
    return err_code;
}
