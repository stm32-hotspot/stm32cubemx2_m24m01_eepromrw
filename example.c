/**
  ******************************************************************************
  * file           : example.c
  * brief          : example program body
  ******************************************************************************
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "example.h"
#include "m24m01.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

#define SIZE256    256U                    /* Size used for standard R/W buffer operations            */
#define LOCK_ID_PAGE_EXECUTE 0U            /* Set to 1 to execute permanent ID page lock procedure    */
/* Private variables ---------------------------------------------------------*/
m24m01_object_t *pM24m010;                 /* Pointer to the M24M01 driver object instance            */
/* Sample data for R/W */
uint8_t tx256[SIZE256] = "abcdefghE-EEPROM-Expansion Firmware library EEPROM driver example : This firmware provides "
                         "a basic example of how to use the X-Nucleo-eXpansion firmware library. This block of data is "
                         "specially written to test the data write function of EEPROM (SPI/I2C)  ";
uint8_t rx_buff[300] = {0};                /* Receive buffer used in data/page tests                  */
uint8_t Transmit_Buff[256] = {0};          /* Buffer used to test EEPROM content                      */

/* Private function prototypes -----------------------------------------------*/
#if DEBUG_APPLI
#define PRINTF_APPLI(...) printf(__VA_ARGS__)   /* Debug print macro when DEBUG_APPLI is enabled    */
#else
#define PRINTF_APPLI(...)                       /* Debug print disabled                             */
#endif /* DEBUG_APPLI */

/**
  * ########## Step 1 ##########
  * The init of M24M01 is triggered by the application code
  */
app_status_t app_init(void)
{
  app_status_t return_status = EXEC_STATUS_ERROR;
  /* Get M24M01 object pointer from BSP or driver layer */
  pM24m010 = MX_M24M01_getobject();
  /* Initialize M24M01 device instance 0 */
  if (m24m01_drv_init(pM24m010, MX_M24M01) != 0)
  {
    PRINTF("[ERROR] Step 1: M24M01 EEPROM init error\r\n");
    goto _app_init_exit;
  }
  PRINTF("[INFO] Step 1: M24M01 EEPROM init completed\r\n");

  /* Initialization completed successfully */
  return_status = EXEC_STATUS_INIT_OK;

_app_init_exit:
  /* Return application-level init status */
  return return_status;
}

/**
  * ########## Step 2 ##########
  * Perform Read and Write operations.
  * The values are displayed on the terminal.
  * output: EXEC_STATUS_OK if OK, EXEC_STATUS_ERROR in case of error
  */
app_status_t app_process(void)
{
  app_status_t return_status = EXEC_STATUS_OK;

  /* Single-byte read/write test */
  app_status_t return_status_sb = M24M01TestSingleByte();

  /* Multi-byte buffer read/write test */
  app_status_t return_status_td = M24M01TestData();

  /* Page read/write test */
  app_status_t return_status_tp = M24M01TestPage();

#if (M24M01_ID_PAGE_ENABLE == 1)
  /* ID page read/write test */
  app_status_t return_status_tidp = M24M01TestIDPage();

  /* Optional permanent ID page lock (controlled by LOCK_ID_PAGE_EXECUTE) */
  app_status_t return_status_lidp = M24M01LockIDPage();
#else
  /* ID page features not enabled at compile time */
  app_status_t return_status_tidp = EXEC_STATUS_OK;
  app_status_t return_status_lidp = EXEC_STATUS_OK;
#endif /* M24M01_ID_PAGE_ENABLE */

  /* Aggregate status of all executed tests */
  if ((return_status_sb == EXEC_STATUS_OK)
      && (return_status_td == EXEC_STATUS_OK)
      && (return_status_tp == EXEC_STATUS_OK)
      && (return_status_tidp == EXEC_STATUS_OK)
      && (return_status_lidp == EXEC_STATUS_OK))
  {
    PRINTF("\n\nAll test cases PASSED.\r\n");
    return_status = EXEC_STATUS_OK;
  }
  else
  {
    PRINTF("\n\nTest cases FAILED.\r\n");
    return_status = EXEC_STATUS_ERROR;
  }

  return return_status;
}

/** ########## Step 3 ##########
  * In this example, app_deinit is never called and is provided as a reference only.
  */
app_status_t app_deinit(void)
{
  /* Deinitialize the M24M01 driver and associated resources */
  if (m24m01_drv_deinit(pM24m010) != 0)
  {
    PRINTF("[ERROR] Step 3: EEPROM deinit error\r\n");
    return EXEC_STATUS_ERROR;
  }

  return EXEC_STATUS_OK;
}


/**
  * @brief  Test single byte read/write on M24M01.
  *         Writes one byte, reads it back and compares the value.
  * @param  None
  * @retval app_status_t
  */
app_status_t M24M01TestSingleByte(void)
{

  app_status_t ret_val = EXEC_STATUS_OK;

  PRINTF("\n\n***************************************************************\r\n");
  PRINTF("                   -- I2C EEPROM TEST SINGLE BYTE-- \r\n");
  PRINTF("***************************************************************\r\n");

  uint8_t tx = 0x89;               /* Byte value to be programmed                                 */
  uint8_t rx = 0xFF;               /* Byte value read back from memory                           */
  unsigned int target_addr = 0x00;     /* Test address used for single-byte access                   */
  uint16_t nbyte = 1;              /* Number of bytes for the read operation                     */

  /* Read current content at target address */
  if (m24m01_drv_read_data_addr16(pM24m010, &rx, target_addr, nbyte) == 0)
  {
    PRINTF("Read Memory Data : 0x%x at Address : 0x%x\r\n", rx, target_addr);
  }

  /* Write single byte to EEPROM */
  int32_t w_ret = m24m01_drv_write_byte_addr16(pM24m010, &tx, target_addr);

  /* Read back the same location */
  int32_t r_ret = m24m01_drv_read_data_addr16(pM24m010, &rx, target_addr, nbyte);

  /* Evaluate write and read results */
  if ((w_ret == 0) && (r_ret == 0))
  {
    if (rx == tx)
    {
      PRINTF("TestByte | Target: %s | Address: 0x%u | TX: 0x%x | RX: 0x%x | Result: PASSED \r\n",
             "M24M01", target_addr, tx, rx);
    }
    else
    {
      PRINTF("TestByte | Target: %s| Address: %u | Result: FAILED \r\n", "M24M01", target_addr);
      ret_val = EXEC_STATUS_ERROR;
    }
  }
  else
  {
    PRINTF("TestByte | Target: %s| Write or Read Operation FAILED \r\n", "M24M01");
    ret_val = EXEC_STATUS_ERROR;
  }

  return ret_val;
}

/**
  * @brief  Test buffered data read/write on M24M01.
  *         Writes 256 bytes, verifies content, then clears them to 0xFF.
  * @param  None
  * @retval app_status_t
  */
app_status_t M24M01TestData(void)
{

  app_status_t ret_val = EXEC_STATUS_OK;
  uint16_t idx;

  PRINTF("\n\n***************************************************************\r\n");
  PRINTF("                   -- I2C EEPROM TEST DATA-- \r\n");
  PRINTF("***************************************************************\r\n");

  unsigned int target_addr = 0x00;       /* Start address for buffer test                             */
  memset(rx_buff, 0x00, sizeof(rx_buff));

  /* Display memory content before write */
  PRINTF("\n\nMemory contents before write (256 bytes): \r\n");
  if (m24m01_drv_read_data_addr16(pM24m010, rx_buff, target_addr, SIZE256) == 0)
  {
    for (idx = 0; idx < SIZE256; idx++)
    {
      PRINTF("0x%x ", rx_buff[idx]);
    }
  }

  /* Write 256 bytes from tx256 buffer */
  int32_t w_ret = m24m01_drv_write_data_addr16(pM24m010, tx256, target_addr, M24_PAGE_SIZE, SIZE256);

  /* Read back into rx_buff (up to its full size) */
  int32_t r_ret = m24m01_drv_read_data_addr16(pM24m010, rx_buff, target_addr, sizeof(rx_buff));

  if ((w_ret == 0) && (r_ret == 0))
  {
    PRINTF("\n\nMemory contents after write: \r\n");
    /* Compare content byte-by-byte against original tx256 buffer */
    for (idx = 0; idx < sizeof(rx_buff); idx++)
    {
      if (tx256[idx] == rx_buff[idx])
      {
        PRINTF("%c", rx_buff[idx]);
      }
      else
      {
        break;
      }
    }

    if (idx == SIZE256)
    {
      PRINTF("\nAll data to M24M01 written successfully!\r\n");
    }
    else
    {
      PRINTF("Error in M24M01 write.\r\n");
      return EXEC_STATUS_ERROR;
    }

    /* Clear test area to 0xFF to restore initial state */
    target_addr = 0;
    PRINTF("\nReset memory to 0xFF from Address:0x%2.2X \r\n", target_addr);
    memset(Transmit_Buff, 0xFF, sizeof(Transmit_Buff));

    if (m24m01_drv_write_data_addr16(pM24m010, Transmit_Buff, target_addr, M24_PAGE_SIZE, SIZE256) != 0)
    {
      ret_val = EXEC_STATUS_ERROR;
    }
    else
    {
      /* Read back cleared data for confirmation */
      memset(rx_buff, 0x00, sizeof(rx_buff));
      if (m24m01_drv_read_data_addr16(pM24m010, rx_buff, target_addr, SIZE256) == 0)
      {
        for (idx = 0; idx < SIZE256; idx++)
        {
          PRINTF("0x%x ", rx_buff[idx]);
        }
      }
      PRINTF("\nMemory contents of M24M01 cleared to 0xFF \r\n");
    }

  }
  else
  {
    PRINTF("M24M01 Test Memory Data: FAILED \r\n");
    ret_val = EXEC_STATUS_ERROR;
  }

  return ret_val;
}


/**
  * @brief  Test page-oriented read/write on M24M01.
  *         Writes one page, verifies content, then clears it to 0xFF.
  * @param  None
  * @retval app_status_t
  */
app_status_t M24M01TestPage(void)
{

  PRINTF("\n\n***************************************************************\r\n");
  PRINTF("                   -- I2C EEPROM TEST PAGE-- \r\n");
  PRINTF("***************************************************************\r\n");

  app_status_t ret_val = EXEC_STATUS_OK;
  unsigned int idx;
  uint8_t tx[M24_PAGE_SIZE] = {0};   /* Local page-sized transmit buffer                          */
  uint8_t rx[M24_PAGE_SIZE] = {0};   /* Local page-sized receive buffer                           */
  unsigned int target_addr = 0x00;       /* Base address of the test page                             */
  uint16_t nbyte = M24_PAGE_SIZE;    /* Number of bytes for page access                           */

  /* Fill transmit buffer with fixed pattern 0x45 */
  memset(tx, 0x45, M24_PAGE_SIZE);

  /* Program one page at address 0x0000 */
  int32_t w_ret = m24m01_drv_write_page_addr16(pM24m010, tx, target_addr, nbyte);

  /* Read back the same page */
  int32_t r_ret = m24m01_drv_read_page_addr16(pM24m010, rx, target_addr, nbyte);

  if ((w_ret == 0) && (r_ret == 0))
  {
    /* Compare written and read data, printing values while they match */
    for (idx = 0; idx < M24_MEMORY_SIZE; idx++) /* Intentionally loop beyond one page for robustness */
    {
      if (tx[idx] == rx[idx])
      {
        PRINTF("0x%x ", rx[idx]);
      }
      else
      {
        break;
      }
    }

    if (idx == M24_PAGE_SIZE)
    {
      PRINTF("\nAll data to M24M01 written successfully!\r\n");
    }
    else
    {
      PRINTF("Error in M24M01 write.\r\n");
      return EXEC_STATUS_ERROR;
    }

    /* Clear tested page to 0xFF */
    target_addr = 0;
    PRINTF("\nReset memory to 0xFF from Address:0x%2.2X \r\n", target_addr);
    memset(tx, 0xFF, sizeof(tx));

    if (m24m01_drv_write_page_addr16(pM24m010, tx, target_addr, M24_PAGE_SIZE) != 0)
    {
      ret_val = EXEC_STATUS_ERROR;
    }
    else
    {
      PRINTF("\nMemory contents of M24M01 cleared to 0xFF \r\n");
    }
  }
  else
  {
    PRINTF("Test PAGE | Target: %s| Write or Read Operation FAILED \r\n", "M24M01");
    ret_val = EXEC_STATUS_ERROR;
  }

  return ret_val;
}

#if (M24M01_ID_PAGE_ENABLE == 1)
/**
  * @brief  Test ID page read/write on M24M01.
  *         Dumps ID page, writes a pattern, verifies and then clears it.
  * @param  None
  * @retval app_status_t
  */
app_status_t M24M01TestIDPage(void)
{

  PRINTF("\n\n***************************************************************\r\n");
  PRINTF("                   -- I2C EEPROM TEST ID PAGE-- \r\n");
  PRINTF("***************************************************************\r\n");

  app_status_t ret_val = EXEC_STATUS_OK;
  unsigned int idx;
  uint8_t tx[M24_PAGE_SIZE] = {0};   /* Transmit buffer for ID page                               */
  uint8_t rx[M24_PAGE_SIZE] = {0};   /* Receive buffer for ID page                                */
  unsigned int target_addr = 0x00;       /* Base address in ID page                                   */
  uint16_t nbyte = M24_PAGE_SIZE;    /* Number of bytes used in ID page operations                */

  /* Prepare ID page pattern 0x55 */
  memset(tx, 0x55, M24_PAGE_SIZE);

  /* Read ID page before write and print current content */
  PRINTF("M24M01 ID page before write \r\n");
  if (m24m01_drv_read_idpage_addr16(pM24m010, rx, target_addr, nbyte) == 0)
  {
    for (idx = 0; idx < M24_PAGE_SIZE; idx++)
    {
      PRINTF("0x%x ", rx[idx]);
    }
  }
  else
  {
    return EXEC_STATUS_ERROR;
  }

  /* Write ID page with test pattern */
  int32_t w_ret = m24m01_drv_write_idpage_addr16(pM24m010, tx, target_addr, nbyte);

  /* Read back ID page content */
  int32_t r_ret = m24m01_drv_read_idpage_addr16(pM24m010, rx, target_addr, nbyte);
  PRINTF("\n\nM24M01 ID page after write \r\n");
  if ((w_ret == 0) && (r_ret == 0))
  {
    for (idx = 0; idx < M24_MEMORY_SIZE; idx++)
    {
      if (tx[idx] == rx[idx])
      {
        PRINTF("0x%x ", rx[idx]);
      }
      else
      {
        break;
      }
    }

    if (idx == M24_PAGE_SIZE)
    {
      PRINTF("\nAll data to M24M01 ID Page written successfully!\r\n");
    }
    else
    {
      PRINTF("Error in M24M01 ID Page write.\r\n");
      return EXEC_STATUS_ERROR;
    }

    /* Clear ID page back to 0xFF after test */
    target_addr = 0;
    PRINTF("\nReset ID Page memory to 0xFF from Address:0x%2.2X \r\n", target_addr);
    memset(tx, 0xFF, sizeof(tx));

    if (m24m01_drv_write_idpage_addr16(pM24m010, tx, target_addr, M24_PAGE_SIZE) != 0)
    {
      ret_val = EXEC_STATUS_ERROR;
    }
    else
    {
      PRINTF("\nMemory contents of M24M01 ID Page cleared to 0xFF \r\n");
    }
  }
  else
  {
    PRINTF("Test ID Page PAGE | Target: %s| Write or Read Operation FAILED \r\n", "M24M01");
    ret_val = EXEC_STATUS_ERROR;
  }

  return ret_val;
}

/**
  * @brief  Lock the M24M01 ID page permanently.
  *         Execution controlled by LOCK_ID_PAGE_EXECUTE compile-time switch.
  * @param  None
  * @retval app_status_t
  */
app_status_t M24M01LockIDPage(void)
{
  PRINTF("\n\n***************************************************************\r\n");
  PRINTF("          -- I2C EEPROM LOCK TEST ID PAGE PERMANENTLY -- \r\n");
  PRINTF("***************************************************************\r\n");

  app_status_t ret_val = EXEC_STATUS_UNKNOWN;
#if (!LOCK_ID_PAGE_EXECUTE)
  /* ID page lock feature disabled at compile time */
  PRINTF("[WARN] Enable Lock ID Page function in Application to execute! \r\n");
  ret_val = EXEC_STATUS_OK;
#else
  /* Request permanent lock of the ID page through driver */
  if (m24m01_drv_lock_idpage(pM24m010) != 0)
  {
    ret_val = EXEC_STATUS_ERROR;
  }
  else
  {
    ret_val = EXEC_STATUS_OK;
  }
#endif /* LOCK_ID_PAGE_EXECUTE */

  return ret_val;

}

#endif /* #if (M24M01_ID_PAGE_ENABLE == 1) */
