/****************************************************************
 * Memory map configuration for using application with MCU-boot *
 ****************************************************************/
/*
 * Internal nRF52 flash memory:
 * 0x00000000: MCUBoot(28 kB)
 * 0x00007000: Reboot logs (4 kB)
 * 0x00008000: MCUBoot header (32 bytes)
 * 0x00008020: Application (463+ kB)
 * 0x0007be50: MCUBoot image trailer (432 bytes)
 * 0x0007c000: MCUBoot Scratch partition (4 kB)
 * 0x0007d000: unused (12 kB)
 *
 * SPI flash:
 * 0x00000000: Bootloader Assets, like Boot Graphic (256 kB)
 * 0x00040000: Application 2 (including MCUBoot header) (464 kB)
 * 0x000b4000: User files - littlefs (3376 kB)
 *
 * This linker script sets the origin of the application at 0x00
 * which means that the application must be linked with a start
 * address at 0x00 and that it'll overwrite the bootloader.
 * This is very practical when debugging and testing on
 * the hardware with a SWD probe.
 */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MCUBOOT_SIZE = 0x8000;
SCRATCH_SIZE = 0x1000;
TLV_SIZE =  0x28; /* Metadata added by imgtool at the end of the image */
SCRATCH_OFFSET = 0x7c000;
MCUBOOT_APP_IMAGE_HEADER_SIZE = 0x20;
MCUBOOT_APP_IMAGE_TRAILER_SIZE = 0x1b0;
APP_OFFSET = 0x00;
APP_SIZE = SCRATCH_OFFSET - MCUBOOT_SIZE - MCUBOOT_APP_IMAGE_HEADER_SIZE - MCUBOOT_APP_IMAGE_TRAILER_SIZE - TLV_SIZE;

MEMORY
{
  /* MCUBOOT (r) : ORIGIN = 0x0, LENGTH = MCUBOOT_SIZE */
  FLASH (rx) : ORIGIN = APP_OFFSET, LENGTH = APP_SIZE
  /* SCRATCH (r) : ORIGIN = SCRATCH_OFFSET, LENGTH = SCRATCH_SIZE */
  SPARE_SPACE (r) : ORIGIN = SCRATCH_OFFSET + SCRATCH_SIZE, LENGTH = 12K
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 64K
}

SECTIONS
{
  .noinit(NOLOAD):
  {
    PROVIDE(__start_noinit_data = .);
    *(.noinit)
    PROVIDE(__stop_noinit_data = .);
  } > RAM
} INSERT AFTER .bss

SECTIONS
{
  . = ALIGN(4);
  .mem_section_dummy_ram :
  {
  }
  .cli_sorted_cmd_ptrs :
  {
    PROVIDE(__start_cli_sorted_cmd_ptrs = .);
    KEEP(*(.cli_sorted_cmd_ptrs))
    PROVIDE(__stop_cli_sorted_cmd_ptrs = .);
  } > RAM
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
  .log_dynamic_data :
  {
    PROVIDE(__start_log_dynamic_data = .);
    KEEP(*(SORT(.log_dynamic_data*)))
    PROVIDE(__stop_log_dynamic_data = .);
  } > RAM
  .log_filter_data :
  {
    PROVIDE(__start_log_filter_data = .);
    KEEP(*(SORT(.log_filter_data*)))
    PROVIDE(__stop_log_filter_data = .);
  } > RAM


} INSERT AFTER .data;

SECTIONS
{
  .mem_section_dummy_rom :
  {
  }
  .sdh_soc_observers :
  {
    PROVIDE(__start_sdh_soc_observers = .);
    KEEP(*(SORT(.sdh_soc_observers*)))
    PROVIDE(__stop_sdh_soc_observers = .);
  } > FLASH
  .sdh_ble_observers :
  {
    PROVIDE(__start_sdh_ble_observers = .);
    KEEP(*(SORT(.sdh_ble_observers*)))
    PROVIDE(__stop_sdh_ble_observers = .);
  } > FLASH
  .sdh_req_observers :
  {
    PROVIDE(__start_sdh_req_observers = .);
    KEEP(*(SORT(.sdh_req_observers*)))
    PROVIDE(__stop_sdh_req_observers = .);
  } > FLASH
  .sdh_state_observers :
  {
    PROVIDE(__start_sdh_state_observers = .);
    KEEP(*(SORT(.sdh_state_observers*)))
    PROVIDE(__stop_sdh_state_observers = .);
  } > FLASH
  .sdh_stack_observers :
  {
    PROVIDE(__start_sdh_stack_observers = .);
    KEEP(*(SORT(.sdh_stack_observers*)))
    PROVIDE(__stop_sdh_stack_observers = .);
  } > FLASH
    .nrf_queue :
  {
    PROVIDE(__start_nrf_queue = .);
    KEEP(*(.nrf_queue))
    PROVIDE(__stop_nrf_queue = .);
  } > FLASH
    .nrf_balloc :
  {
    PROVIDE(__start_nrf_balloc = .);
    KEEP(*(.nrf_balloc))
    PROVIDE(__stop_nrf_balloc = .);
  } > FLASH
    .cli_command :
  {
    PROVIDE(__start_cli_command = .);
    KEEP(*(.cli_command))
    PROVIDE(__stop_cli_command = .);
  } > FLASH
  .crypto_data :
  {
    PROVIDE(__start_crypto_data = .);
    KEEP(*(SORT(.crypto_data*)))
    PROVIDE(__stop_crypto_data = .);
  } > FLASH
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(SORT(.pwr_mgmt_data*)))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > FLASH
  .log_const_data :
  {
    PROVIDE(__start_log_const_data = .);
    KEEP(*(SORT(.log_const_data*)))
    PROVIDE(__stop_log_const_data = .);
  } > FLASH
  .log_backends :
  {
    PROVIDE(__start_log_backends = .);
    KEEP(*(SORT(.log_backends*)))
    PROVIDE(__stop_log_backends = .);
  } > FLASH
    .nrf_balloc :
  {
    PROVIDE(__start_nrf_balloc = .);
    KEEP(*(.nrf_balloc))
    PROVIDE(__stop_nrf_balloc = .);
  } > FLASH

} INSERT AFTER .text


INCLUDE "./nrf_common.ld"
