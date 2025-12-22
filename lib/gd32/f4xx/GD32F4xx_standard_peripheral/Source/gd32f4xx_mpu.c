/*!
    \file  mpu.c
    \brief mpu driver
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-08-15, V1.0.2, firmware for GD32F4xx
*/

#include "gd32f4xx.h"
#include "gd32f4xx_mpu.h"

// mpu config bootloader
#define ALLREGION_BASE_ADDRESS (0x00000000UL)
#define ALLREGION_SIZE MPU_REGION_SIZE_4GB

#define BOOT_RAM0_BASE_ADDRESS (0x20000000UL)
#define BOOT_RAM0_SIZE MPU_REGION_SIZE_128KB

#define BOOT_RAM1_BASE_ADDRESS (0x20020000UL)
#define BOOT_RAM1_SIZE MPU_REGION_SIZE_64KB

// #define BOOT_FLASH_FORBIDDEN_PERMISSION_ADDRESS \
//   (0x08007FE0UL)  // 32B never region

// #define BOOT_FLASH_FORBIDDEN_PERMISSION_ADDRESS \
//   (0x0800F000UL)  // 32B never region
// #define BOOT_FLASH_FORBIDDEN_PERMISSION_SIZE MPU_REGION_SIZE_32B

#define BOOT_PERIPH0_BASE_ADDRESS (0x40000000UL)
#define BOOT_PERIPH0_SIZE MPU_REGION_SIZE_128MB

#define BOOT_PERIPH1_BASE_ADDRESS (0x40020000UL)
#define BOOT_PERIPH1_SIZE MPU_REGION_SIZE_16KB

#define BOOT_PERIPH_DMA_BASE_ADDRESS (0x40026000UL)
#define BOOT_PERIPH_DMA_SIZE MPU_REGION_SIZE_8KB

// mpu config firmware
#define FIRM_RAM0_BASE_ADDRESS (0x20000000UL)
#define FIRM_RAM0_SIZE MPU_REGION_SIZE_128KB
#define FIRM_RAM1_BASE_ADDRESS (0x20020000UL)
#define FIRM_RAM1_SIZE MPU_REGION_SIZE_64KB

#define FIRM_FLASH0_BASE_ADDRESS (0x08000000UL)
#define FIRM_FLASH0_SIZE MPU_REGION_SIZE_1MB
#define FIRM_FLASH1_BASE_ADDRESS (0x08100000UL)
#define FIRM_FLASH1_SIZE MPU_REGION_SIZE_2MB

#define FIRM_PERIPH_BASE_ADDRESS (0x40000000UL)
#define FIRM_PERIPH_SIZE MPU_REGION_SIZE_512MB

#define FIRM_PERIPH_DMA_BASE_ADDRESS (0x40026000UL)
#define FIRM_PERIPH_DMA_SIZE MPU_REGION_SIZE_8KB

#define FIRM_PERIPH_SYSCFG_BASE_ADDRESS (0x40013800UL)
#define FIRM_PERIPH_SYSCFG_SIZE MPU_REGION_SIZE_1KB

//// for test memmanage handler
// #define TEST0_ARRAY_BASE_ADDRESS           (RAM1_BASE_ADDRESS + RAM1_SIZE +
//  0x1000) #define TEST0_ARRAY_SIZE                   MPU_REGION_SIZE_32B

// #define TEST1_ARRAY_BASE_ADDRESS           (FLASH1_BASE_ADDRESS + FLASH1_SIZE
//+ 	0x1000) #define TEST1_ARRAY_SIZE                   MPU_REGION_SIZE_32B

// #ifdef __CC_ARM
//  uint8_t PrivilegedReadOnlyArray[32]   __attribute__( ( at( 0x20030000 ) ) );
////uint8_t PrivilegedReadOnlyArray1[32]   __attribute__( ( at( 0x20003000 ) )
///);
// #endif
// #ifdef __IAR_SYSTEMS_ICC__
//__no_init uint8_t PrivilegedReadOnlyArray[32] @0x20002000;
// #endif
//  uint8_t read_data;

/*!
    \brief      test core support MPU or not
    \param[in]  none

    \param[out] none
    \retval     FALSE or TRUE
*/
bool mpu_exitent(void) {
  if (MPU_NOT_EXISTENT == MPU_TEST) {
    /* core does not support mpu */
    return false;

  } else {
    /* core support mpu */
    return true;
  }
}
/*!
    \brief      enable mpu
    \param[in]  mpu_control: MPU_HFNMI_DISABLE_PRIVDEF_DISABLE,
   MPU_HFNMI_ENABLE_PRIVDEF_DISABLE, MPU_HFNMI_DISABLE_PRIVDEF_ENABLE,
   MPU_HFNMI_ENABLE_PRIVDEF_ENABLE \param[out] none \retval     none
*/
void mpu_enable(uint32_t mpu_control) {
  /* enable the mpu */
  MPU->CTRL = mpu_control | MPU_CTRL_ENABLE_Msk;

  /* enable fault exceptions */
  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

  /* ensure mpu setting take effects */
  __DSB();
  __ISB();
}

/*!
    \brief      disable mpu
    \param[in]  none

    \param[out] none
    \retval     none
*/
void mpu_disable(void) {
  /* make sure outstanding transfers are done */
  __DMB();

  /* disable fault exceptions */
  SCB->SHCSR &= ~SCB_SHCSR_MEMFAULTENA_Msk;

  /* disable the mpu and clear the control register*/
  MPU->CTRL = 0U;
}

/*!
    \brief      configure mpu region
    \param[in]  mpu_init: MPU region parameter initialization stuct members of
   the structure and the member values are shown as below: number:
   MPU_REGION_NUMBER0, MPU_REGION_NUMBER1 MPU_REGION_NUMBER2, MPU_REGION_NUMBER3
                          MPU_REGION_NUMBER4, MPU_REGION_NUMBER5
                          MPU_REGION_NUMBER6, MPU_REGION_NUMBER7
                  instruction_accessable: MPU_INSTRUCTION_ACCESS_ENABLE,
   MPU_INSTRUCTION_ACCESS_DISABLE access_permission:
   MPU_REGION_PRIV_DISABLE_USER_DISABLE, MPU_REGION_PRIV_READ_WRITE
                                     MPU_REGION_PRIV_READ_WRITE_USER_READ_ONLY,
   MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE MPU_REGION_PRIV_READ_ONLY,
   MPU_REGION_PRIV_READ_ONLY_USER_READ_ONLY type_extension_field:
   MPU_TEX_LEVEL0, MPU_TEX_LEVEL1, MPU_TEX_LEVEL2 shareable:
   MPU_ACCESS_SHAREABLE, MPU_ACCESS_NOT_SHAREABLE cacheable:
   MPU_ACCESS_CACHEABLE, MPU_ACCESS_NOT_CACHEABLE bufferable:
   MPU_ACCESS_BUFFERABLE, MPU_ACCESS_NOT_BUFFERABLE sub_region_disable:
   MPU_SUB_REGION_ALL_DISABLE, MPU_SUB_REGION_0_ENABLE MPU_SUB_REGION_1_ENABLE,
   MPU_SUB_REGION_2_ENABLE MPU_SUB_REGION_3_ENABLE, MPU_SUB_REGION_4_ENABLE
                                      MPU_SUB_REGION_5_ENABLE,
   MPU_SUB_REGION_6_ENABLE, MPU_SUB_REGION_6_ENABLE region_size:
   MPU_REGION_SIZE_32B, MPU_REGION_SIZE_64B, MPU_REGION_SIZE_128B,
   MPU_REGION_SIZE_256B, MPU_REGION_SIZE_512B MPU_REGION_SIZE_1KB,
   MPU_REGION_SIZE_2KB, MPU_REGION_SIZE_4KB, MPU_REGION_SIZE_8KB,
   MPU_REGION_SIZE_16KB MPU_REGION_SIZE_32KB, MPU_REGION_SIZE_64KB,
   MPU_REGION_SIZE_128KB, MPU_REGION_SIZE_256KB, MPU_REGION_SIZE_512KB
                               MPU_REGION_SIZE_1MB, MPU_REGION_SIZE_2MB,
   MPU_REGION_SIZE_4MB, MPU_REGION_SIZE_8MB, MPU_REGION_SIZE_16MB,
   MPU_REGION_SIZE_32MB MPU_REGION_SIZE_64MB, MPU_REGION_SIZE_128MB,
   MPU_REGION_SIZE_256MB, MPU_REGION_SIZE_512MB MPU_REGION_SIZE_1GB,
   MPU_REGION_SIZE_2GB, MPU_REGION_SIZE_4GB enable: MPU_REGION_ENABLE,
   MPU_REGION_DISABLE \param[out] none \retval     none
*/
void mpu_region_config(mpu_region_init_struct *mpu_init) {
  /* set the region number */
  MPU->RNR = mpu_init->number;

  /* enable the region and configure its parameter */
  if (ENABLE == (mpu_init->enable)) {
    MPU->RBAR = mpu_init->base_address;
    MPU->RASR =
        ((uint32_t)mpu_init->instruction_accessable << MPU_RASR_XN_Pos) |
        ((uint32_t)mpu_init->access_permission << MPU_RASR_AP_Pos) |
        ((uint32_t)mpu_init->type_extension_field << MPU_RASR_TEX_Pos) |
        ((uint32_t)mpu_init->shareable << MPU_RASR_S_Pos) |
        ((uint32_t)mpu_init->cacheable << MPU_RASR_C_Pos) |
        ((uint32_t)mpu_init->bufferable << MPU_RASR_B_Pos) |
        ((uint32_t)mpu_init->sub_region_disable << MPU_RASR_SRD_Pos) |
        ((uint32_t)mpu_init->region_size << MPU_RASR_SIZE_Pos) |
        ((uint32_t)mpu_init->enable << MPU_RASR_ENABLE_Pos);

  } else {
    /* disable the region */
    MPU->RBAR = 0x00U;
    MPU->RASR = 0x00U;
  }
}
/*!
    \brief      setup mpu regions
    \param[in]  none

    \param[out] none
    \retval     none
*/
void mpu_setup_boot_region(void) {
  mpu_region_init_struct mpu_init_struct;

  if (!mpu_exitent()) return;

  /* disable MPU */
  mpu_disable();

  /* configure Everything */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = ALLREGION_BASE_ADDRESS;  //
  mpu_init_struct.region_size =
      ALLREGION_SIZE;  // 0x00000000 - 0xFFFFFFFF, 4 GiB, read-write
  mpu_init_struct.access_permission =
      MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE;
  mpu_init_struct.bufferable = MPU_ACCESS_NOT_BUFFERABLE;
  mpu_init_struct.cacheable = MPU_ACCESS_NOT_CACHEABLE;
  mpu_init_struct.shareable = MPU_ACCESS_NOT_SHAREABLE;
  mpu_init_struct.number = MPU_REGION_NUMBER_0;
  mpu_init_struct.type_extension_field = MPU_TEX_LEVEL_0;
  mpu_init_struct.sub_region_disable = MPU_SUB_REGION_ALL_DISABLE;
  mpu_init_struct.instruction_accessable = MPU_INSTRUCTION_ACCESS_ENABLE;
  mpu_region_config(&mpu_init_struct);

  // /* configure FLASH forbidden permission region as region 1, 32B */
  // mpu_init_struct.enable = MPU_REGION_ENABLE;
  // mpu_init_struct.base_address = BOOT_FLASH_FORBIDDEN_PERMISSION_ADDRESS;
  // mpu_init_struct.region_size =
  //     BOOT_FLASH_FORBIDDEN_PERMISSION_SIZE;  // 0x8007FE0 - 0x08007FFF 32B
  //                                            // no-access
  // mpu_init_struct.access_permission = MPU_REGION_PRIV_DISABLE_USER_DISABLE;
  // mpu_init_struct.number = MPU_REGION_NUMBER_1;
  // mpu_region_config(&mpu_init_struct);

  /* configure RAM0 region as region 1, 128kB of size and R/W region */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = BOOT_RAM0_BASE_ADDRESS;
  mpu_init_struct.region_size = BOOT_RAM0_SIZE;  // 0x20000000 ~ 0x20020000
  mpu_init_struct.access_permission =
      MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE;
  mpu_init_struct.number = MPU_REGION_NUMBER_1;
  mpu_region_config(&mpu_init_struct);

  /* configure RAM1 region as region 2, 64kB of size and R/W region */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = BOOT_RAM1_BASE_ADDRESS;
  mpu_init_struct.region_size = BOOT_RAM1_SIZE;  // 0x20020000 ~ 0x20030000
  mpu_init_struct.access_permission =
      MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE;
  mpu_init_struct.number = MPU_REGION_NUMBER_2;
  mpu_region_config(&mpu_init_struct);

  /* configure peripheral0 region as region 3, 128MB of size, R/W and
  execute
   * never region */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = BOOT_PERIPH0_BASE_ADDRESS;
  mpu_init_struct.region_size = BOOT_PERIPH0_SIZE;
  mpu_init_struct.access_permission =
      MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE;
  mpu_init_struct.number = MPU_REGION_NUMBER_3;
  mpu_init_struct.instruction_accessable = MPU_INSTRUCTION_ACCESS_DISABLE;
  mpu_region_config(&mpu_init_struct);

  /* configure peripheral1 region as region 4, 16KB of size, R/W and
  execute
   * never region */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = BOOT_PERIPH1_BASE_ADDRESS;
  mpu_init_struct.region_size = BOOT_PERIPH1_SIZE;
  mpu_init_struct.access_permission =
      MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE;
  mpu_init_struct.number = MPU_REGION_NUMBER_4;
  mpu_init_struct.instruction_accessable = MPU_INSTRUCTION_ACCESS_DISABLE;
  mpu_region_config(&mpu_init_struct);

  /* enable MPU */
  mpu_enable(MPU_HFNMI_DISABLE_PRIVDEF_ENABLE);
}
/*!
    \brief      setup mpu regions
    \param[in]  none

    \param[out] none
    \retval     none
*/
void mpu_setup_firm_region(void) {
  mpu_region_init_struct mpu_init_struct;

  if (!mpu_exitent()) return;

  /* disable MPU */
  mpu_disable();

  /* configure RAM region as region 0, 128kB of size and R/W region */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = FIRM_RAM0_BASE_ADDRESS;  // 0x20000000
  mpu_init_struct.region_size =
      FIRM_RAM0_SIZE;  // 128KB --- 0x20000000 ~ 0x20020000
  mpu_init_struct.access_permission =
      MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE;
  mpu_init_struct.bufferable = MPU_ACCESS_NOT_BUFFERABLE;
  mpu_init_struct.cacheable = MPU_ACCESS_NOT_CACHEABLE;
  mpu_init_struct.shareable = MPU_ACCESS_NOT_SHAREABLE;
  mpu_init_struct.number = MPU_REGION_NUMBER_0;
  mpu_init_struct.type_extension_field = MPU_TEX_LEVEL_0;
  mpu_init_struct.sub_region_disable = MPU_SUB_REGION_ALL_DISABLE;
  mpu_init_struct.instruction_accessable = MPU_INSTRUCTION_ACCESS_ENABLE;

  mpu_region_config(&mpu_init_struct);

  /* configure FLASH region as region 1, 1MB of size and R/W region */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = FIRM_FLASH0_BASE_ADDRESS;
  mpu_init_struct.region_size = FIRM_FLASH0_SIZE;  // 0x08000000 ~ 0x08100000
  mpu_init_struct.access_permission =
      MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE;
  mpu_init_struct.number = MPU_REGION_NUMBER_1;

  mpu_region_config(&mpu_init_struct);

  /* configure RAM region as region 2, 64kB of size and R/W region */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = FIRM_RAM1_BASE_ADDRESS;
  mpu_init_struct.region_size = FIRM_RAM1_SIZE;  // 0x20020000 ~ 0x20030000
  mpu_init_struct.number = MPU_REGION_NUMBER_2;
  mpu_init_struct.access_permission =
      MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE;
  mpu_region_config(&mpu_init_struct);

  /* configure FLASH region as region 3, 1MB of size and R/W region */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = FIRM_FLASH1_BASE_ADDRESS;
  mpu_init_struct.region_size = FIRM_FLASH1_SIZE;  // 0x08100000 ~ 0x08300000
  mpu_init_struct.number = MPU_REGION_NUMBER_3;
  mpu_init_struct.access_permission =
      MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE;
  mpu_region_config(&mpu_init_struct);

  /* configure peripheral region as region 4, 512MB of size, R/W and execute
   * never region */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = FIRM_PERIPH_BASE_ADDRESS;
  mpu_init_struct.region_size = FIRM_PERIPH_SIZE;
  mpu_init_struct.number = MPU_REGION_NUMBER_4;
  mpu_init_struct.access_permission =
      MPU_REGION_PRIV_READ_WRITE_USER_READ_WRITE;
  mpu_init_struct.instruction_accessable = MPU_INSTRUCTION_ACCESS_DISABLE;
  mpu_region_config(&mpu_init_struct);

  /* configure peripheral syscfg region as region 5, 1KB of size, Read only and
   * execute never region */
  mpu_init_struct.enable = MPU_REGION_ENABLE;
  mpu_init_struct.base_address = FIRM_PERIPH_SYSCFG_BASE_ADDRESS;
  mpu_init_struct.region_size = FIRM_PERIPH_SYSCFG_SIZE;
  mpu_init_struct.number = MPU_REGION_NUMBER_5;
  mpu_init_struct.instruction_accessable =
      MPU_REGION_PRIV_READ_ONLY_USER_READ_ONLY;
  mpu_region_config(&mpu_init_struct);

  /* enable MPU */
  mpu_enable(MPU_HFNMI_DISABLE_PRIVDEF_ENABLE);
}
