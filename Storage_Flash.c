//----------------------------------------------------------------------------//
//  Copyright 2023 Clemens Treichler                                          //
//                                                                            //
//  This file is part of ESTL - Embedded Systems Tiny Library,                //
//  see <https://github.com/treichler/ESTL>                                   //
//                                                                            //
//  ESTL is free software: you can redistribute it and/or modify              //
//  it under the terms of the GNU Lesser General Public License as published  //
//  by the Free Software Foundation, either version 3 of the License, or      //
//  (at your option) any later version.                                       //
//                                                                            //
//  ESTL is distributed in the hope that it will be useful,                   //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of            //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              //
//  GNU Lesser General Public License for more details.                       //
//                                                                            //
//  You should have received a copy of the GNU Lesser General Public License  //
//  along with ESTL. If not, see <http://www.gnu.org/licenses/>.              //
//----------------------------------------------------------------------------//

/**
 * @file Storage_Flash.c
 *
 * @author Clemens Treichler clemens.treichler(at)aon.at
 *
 * @copyright Copyright 2023 Clemens Treichler
 *
 * @license Released under the GNU Lesser General Public License
 */


#include <string.h>
#include "ESTL_Types.h"
#include "Error.h"
#include "Crc.h"
#include "Target.h"
#include "Storage.h"


#ifndef NV_MEM_TOTAL_PAGES
  #error "NV_MEM_TOTAL_PAGES has to be defined by Target.h"
#else
  #if( NV_MEM_TOTAL_PAGES < 2)
    #error "Algorithm needs at least two pages."
  #endif
  #if( NV_MEM_TOTAL_PAGES != 2)
    #error "Algorithm currently only works with two pages."
  #endif
#endif

#ifndef NV_MEM_PAGE_SIZE
  #error "NV_MEM_PAGE_SIZE has to be defined by Target.h"
#endif

#ifndef NV_MEM_START_ADDRESS
  #error "NV_MEM_START_ADDRESS has to be defined by Target.h"
#endif

#ifndef NV_MEM_PAGE_EMPTY_VALUE
  #error "NV_MEM_PAGE_EMPTY_VALUE has to be defined by Target.h"
#endif

#ifndef NV_MEM_DATA_ALIGNMENT
  #error "NV_MEM_DATA_ALIGNMENT has to be defined by Target.h"
#else
  #if( (32 != NV_MEM_DATA_ALIGNMENT) && (64 != NV_MEM_DATA_ALIGNMENT) )
    #error "For NV_MEM_DATA_ALIGNMENT only 32 and 64 bit alignment are allowed."
  #endif
#endif

/**
 * @ingroup  STORAGE
 * @defgroup STORAGE_FLASH  Flash-based Storage
 * @brief    Uses MCU's internal flash for non-volatile data storage
 *
 * Flash-based data storage uses alternating pages, which are filled
 * sequentially.
 * A full page is erased as soon as a new page gets written new content and
 * valid data from the page to be erased is moved to the new one.
 *
 * Since writing to flash is hardware dependent to each specific MCU it is
 * necessary to implement its specific flash manipulating functions in Target.c
 * and declare them in Target.h.
 * See below pseudo-code examples for these functions namely Target_NvMemWrite()
 * and Target_NvMemErasePage().
 *
 * @code
 *   error_code_t Target_NvMemWrite( uint16_t page, uint16_t addr, const void * data, uint16_t size )
 *   {
 *     if( ! CheckBoundaries(page, addr, size) )
 *       return INDEX_OUT_OF_BOUNDARY;
 *
 *     uint8_t * d = (uint8_t*)data;
 *     uint8_t * nv_mem = (uint8_t*)(page * NV_MEM_PAGE_SIZE + addr + NV_MEM_START_ADDRESS);
 *     UnlockFlash();
 *     while( size -- )
 *     {
 *       *nv_mem = *d;
 *       nv_mem ++;
 *       d ++;
 *     }
 *     LockFlash();
 *     return OK;
 *   }
 * @endcode
 *
 * @code
 *   error_code_t Target_NvMemErasePage( uint16_t page )
 *   {
 *     if( ! CheckBoundary(page) )
 *       return INDEX_OUT_OF_BOUNDARY;
 *
 *     UnlockFlash();
 *     ErasePage();
 *     LockFlash();
 *     return OK;
 *   }
 * @endcode
 *
 * In Target.h it is also required to provide information regarding flash
 * properties as an example can be found below:
 *
 * @code
 * #define NV_MEM_DATA_ALIGNMENT   (64)         // flash data alignment in bits
 * #define NV_MEM_PAGE_SIZE        (2048)       // flash page size in bytes
 * #define NV_MEM_TOTAL_PAGES      (2)          // currently only 2 pages are supported
 * #define NV_MEM_START_ADDRESS    (0x0801F000) // flash start address for storage
 * #define NV_MEM_PAGE_EMPTY_VALUE (0xFFFFFFFF) // flash content when empty
 * @endcode
 *
 * Read access to flash is expected to be similar to RAM access, same addressing
 * schema.
 * Therefore no dedicated flash read functionality intended.
 *
 * @attention
 * Depending on MCU's architecture it might happen, that during a running
 * debug session the call of any flash writing function could corrupt the
 * accessed page.
 * @{
 */


/**
 * Storage header structure.
 * Its content is arranged to fulfill 64 bit alignment.
 */
typedef struct {
  uint32_t      crc32;          //!<
  uint16_t      size;           //!<  Size of payload, without header
  storage_id_t  index;          //!<  File ID...
  uint8_t       counter;        //!<  Counter to indicate record cycle
} storage_header_t;


/**
 * Current state of a page
 */
typedef enum {
  PAGE_UNINITIALIZED,   //!<  Page is uninitialized
  PAGE_IS_VALID,        //!<  Page is valid
  PAGE_IS_CORRUPTED,    //!<  Page is corrupted
} page_state_t;


/**
 * Status information of a page
 */
typedef struct {
  uint16_t      address;        //!<  Address next free entry
  page_state_t  status;         //!<  Current status related to page's content
} page_status_info_t;


/**
 * Page information non-volatile stored at page.
 */
typedef struct {
  uint16_t erase_counter;       //!<  page erase counter
  uint16_t reserved;            //!<  maybe could be used for CRC or inverted counter...
#if( 64 == NV_MEM_DATA_ALIGNMENT )
  uint32_t dummy;               //!<  in case of 64 bit alignment insert dummy value
#endif
} page_info_t;


struct {
  storage_header_t * headers[NR_OF_STORAGES];           //!<  Pointer to the physical location of each storage
  page_status_info_t pages_info[NV_MEM_TOTAL_PAGES];    //!<  Status information of every page
  uint8_t            active_page;                       //!<  Index of currently active page
  bool_t             is_initialized;                    //!<  Indicator if Storage module is initialized
} StorageFlash_Data;


//direct read access to non-volatile memory
// __IO uint32_t * nv_mem = NV_MEM_START_ADDRESS;
typedef struct {
  uint8_t pages[NV_MEM_TOTAL_PAGES][NV_MEM_PAGE_SIZE];
} nv_mem_t;

#define TARGET_NV_MEM ((nv_mem_t *) NV_MEM_START_ADDRESS)

// The last 4 respectively 8 bytes are reserved for page info
#define NV_MEM_PAGE_INFO_ADDR (NV_MEM_PAGE_SIZE - sizeof(page_info_t))


//-------------------------------------------------------------------------------------------------
//  Local Prototypes
//-------------------------------------------------------------------------------------------------

static inline uint16_t StorageFlash_GetSizeRoundedToFlashBlock( uint16_t byte_size );


/**
 * This function selectively copies provided page's content to the active page.
 * Only data that is initialized successfully and located in the provided page
 * is affected, so this is not a deep-copy.
 *
 * @param page          This page's content is selectively copied
 * @return
 *   @retval    FALSE   Data could not be copied due to lack of left space.
 *   @retval    TRUE    Data is copied respectively there was no data to copy.
 */
static bool_t StorageFlash_MigrateToActivePage( uint8_t page );


/**
 * Erase whole page and refill it only with currently active storage entries
 * that are located within this page.
 * Active data is temporarily saved to RAM before re-writing to page.
 * A power fail during function call might cause data loss.
 *
 * @param page          This page's active content is going to be renewed
 * @return              Error code according to Target_NvMemWrite()
 */
static error_code_t StorageFlash_RenewPage( uint8_t page );


/**
 * Before any Write cycle prepare affected pages, namely the active page and
 * the next page.
 * It is essential to resolve pages which were marked as corrupted during
 * initialization, and to have the next page cleaned up, so it can be used
 * as soon as a page change is necessary due to shortness of remaining space.
 *
 * @param size  The size of the next data to be written to flash
 */
static void StorageFlash_PrepareAffectedPages( uint16_t size );


/**
 * Erase a whole page.
 *
 * @param page  The page to be erased
 */
static void StorageFlash_ErasePage( uint8_t page );


/**
 * Check if a page contains valid data at a particular address.
 * The function does a plausibility check on the header and compares the
 * estimated data checksum with the one provided in the header.
 *
 * @param page
 * @param addr
 * @return
 *   @retval    FALSE   Data is invalid
 *   @retval    TRUE    Data is valid
 */
static bool_t StorageFlash_RecordIsValid( uint16_t page, uint16_t addr );


/**
 * Get the page a dedicated storage header is located at.
 *
 * @param header        The header to be resolved
 * @return              The page where the data is located
 *   @retval    0xFF    In case of an invalid header
 */
static inline uint8_t StorageFlash_HeaderGetPage( storage_header_t * header );


/**
 * Get the address a dedicated storage header is located at.
 *
 * @param header        The header to be resolved
 * @return              The address where the data is located
 *   @retval    0xFFFF  In case of an invalid header
 */
static inline uint16_t StorageFlash_HeaderGetAddress( storage_header_t * header );


/**
 * Get the size of valid and successfully initialized data located in a
 * particular page.
 *
 * @param page          The page to be evaluated
 * @return              Valid and initialized data's size
 */
static uint16_t StorageFlash_PageHoldsValidDataSize( uint8_t page );


//-------------------------------------------------------------------------------------------------
//  Implementation
//-------------------------------------------------------------------------------------------------

/** @cond */
#define BYTES_PER_FLASH_BLOCK   (NV_MEM_DATA_ALIGNMENT / 8)
#define FLASH_BLOCK_SIZE_MASK   (BYTES_PER_FLASH_BLOCK - 1)
/** @endcond */


uint16_t StorageFlash_GetSizeRoundedToFlashBlock( uint16_t byte_size )
{
  if( byte_size & FLASH_BLOCK_SIZE_MASK )
    return (byte_size & ~FLASH_BLOCK_SIZE_MASK) + BYTES_PER_FLASH_BLOCK;
  return byte_size;
}


bool_t StorageFlash_RecordIsValid( uint16_t page, uint16_t addr )
{
  storage_header_t * header = (storage_header_t*)(&TARGET_NV_MEM->pages[page][addr]);
  if( (addr + sizeof(storage_header_t) + header->size) > NV_MEM_PAGE_INFO_ADDR )
    return FALSE;
  if( header->crc32 != Crc_Crc32(((uint8_t*)header) + 4, sizeof(storage_header_t) - 4 + header->size, 0) )
    return FALSE;
  return TRUE;
}


/**
 * See Storage_Init() for further information.
 */
error_code_t StorageFlash_Init( void )
{
  uint16_t address;
  uint8_t page, empty_pages = 0;
  storage_header_t * header;
  for( page = 0; page < NV_MEM_TOTAL_PAGES; page ++ )
  {
    address = 0;
    while( (address < NV_MEM_PAGE_INFO_ADDR) && StorageFlash_RecordIsValid(page, address) )
    {
      header = (storage_header_t*)(&TARGET_NV_MEM->pages[page][address]);

      // check record's index and save pointer to header
      if( (NR_OF_STORAGES > header->index) &&
          (header->crc32 == Crc_Crc32(((uint8_t*)header) + 4, sizeof(storage_header_t) - 4 + header->size, 0)) )
      {
        if( NULL == StorageFlash_Data.headers[header->index] )
          StorageFlash_Data.headers[header->index] = header;
        else
        {
          uint8_t diff = header->counter - StorageFlash_Data.headers[header->index]->counter;
          if( (diff > 0) && (diff <= 127) )
            StorageFlash_Data.headers[header->index] = header;
        }
      }

      // calculate next record's address
      address += ( sizeof(storage_header_t) + StorageFlash_GetSizeRoundedToFlashBlock(header->size) );
    }
    if( (address <= NV_MEM_PAGE_INFO_ADDR) )
    {
      // address now indicates page's data size
      StorageFlash_Data.pages_info[page].address = address;

      StorageFlash_Data.pages_info[page].status = PAGE_IS_VALID;

      if( 0 == address )
        empty_pages ++;

      // page's remaining content has to be empty
      while( address < NV_MEM_PAGE_INFO_ADDR )
      {
        uint32_t * flash_cell = (uint32_t*)(&TARGET_NV_MEM->pages[page][address]);
        if( NV_MEM_PAGE_EMPTY_VALUE != *flash_cell )
        {
          StorageFlash_Data.pages_info[page].status = PAGE_IS_CORRUPTED;
          break;
        }
        address += 4;
      }
    }
    else
      StorageFlash_Data.pages_info[page].status = PAGE_IS_CORRUPTED;
  }

  if( NV_MEM_TOTAL_PAGES == empty_pages )
  {
    // both pages empty: start with page[0] as active page
    StorageFlash_Data.active_page = 0;
  }
  else if( 0 == empty_pages )
  {
    // both pages full: the one with more remaining space is set active
    if( StorageFlash_Data.pages_info[0].address < StorageFlash_Data.pages_info[1].address )
      StorageFlash_Data.active_page = 0;
    else
      StorageFlash_Data.active_page = 1;
  }
  else
  {
    // the page containing data is the active one
    if( 0 != StorageFlash_Data.pages_info[0].address )
      StorageFlash_Data.active_page = 0;
    else
      StorageFlash_Data.active_page = 1;
  }

  StorageFlash_Data.is_initialized = TRUE;
  return OK;
}


uint8_t StorageFlash_HeaderGetPage( storage_header_t * header )
{
  if( NV_MEM_START_ADDRESS <= (uint32_t)header )
    return (uint8_t)(((uint32_t)header - NV_MEM_START_ADDRESS) / NV_MEM_PAGE_SIZE);
  return 0xFF;
}


uint16_t StorageFlash_HeaderGetAddress( storage_header_t * header )
{
  if( NV_MEM_START_ADDRESS <= (uint32_t)header )
    return (uint16_t)(((uint32_t)header - NV_MEM_START_ADDRESS) % NV_MEM_PAGE_SIZE);
  return 0xFFFF;
}

/*
page_info_t * Storage_GetPageCycleCounter( uint8_t page )
{
  return (page_info_t *)(&TARGET_NV_MEM->pages[page][NV_MEM_PAGE_PAYLOAD_SIZE]);
}
*/

uint16_t StorageFlash_PageHoldsValidDataSize( uint8_t page )
{
  storage_id_t i;
  uint16_t size = 0;
  for( i = 0; i < NR_OF_STORAGES; i ++ )
  {
    if( StorageFlash_HeaderGetPage(StorageFlash_Data.headers[i]) == page )
      size += ( sizeof(storage_header_t) + StorageFlash_Data.headers[i]->size );
  }
  return size;
}


void StorageFlash_ErasePage( uint8_t page )
{
  if( NV_MEM_TOTAL_PAGES > page )
  {
    page_info_t * current_page_info = (page_info_t *)(&TARGET_NV_MEM->pages[page][NV_MEM_PAGE_INFO_ADDR]);
    page_info_t new_page_info;
    new_page_info.erase_counter = current_page_info->erase_counter + 1;
    new_page_info.reserved = 0xFFFF;
#if( 64 == NV_MEM_DATA_ALIGNMENT )
    new_page_info.dummy = 0xFFFFFFFF;
#endif

    // TODO eventually handle return code
    Target_NvMemErasePage( page );
    StorageFlash_Data.pages_info[page].status = PAGE_IS_VALID;
    StorageFlash_Data.pages_info[page].address = 0;

    Target_NvMemWrite( page, NV_MEM_PAGE_INFO_ADDR, &new_page_info, sizeof(new_page_info) );
  }
}


bool_t StorageFlash_MigrateToActivePage( uint8_t page )
{
  uint16_t valid_data_size = StorageFlash_PageHoldsValidDataSize(page);
  if( 0 == valid_data_size )
    return TRUE;

  if( valid_data_size > (NV_MEM_PAGE_INFO_ADDR - StorageFlash_Data.pages_info[StorageFlash_Data.active_page].address) )
    return FALSE;

  storage_id_t i;
  for( i = 0; i < NR_OF_STORAGES; i ++ )
  {
    // check if page to be migrated holds current storage entry
    if( StorageFlash_HeaderGetPage(StorageFlash_Data.headers[i]) == page )
    {
      // copy current storage entry to active page
      uint16_t active_page      = StorageFlash_Data.active_page;
      uint16_t active_page_addr = StorageFlash_Data.pages_info[StorageFlash_Data.active_page].address;
      uint16_t size             = StorageFlash_Data.headers[i]->size;
      storage_header_t header;
      header.size    = size;
      header.index   = StorageFlash_Data.headers[i]->index;
      header.counter = StorageFlash_Data.headers[i]->counter + 1;
      uint32_t crc   = Crc_Crc32( (uint8_t*)(&header) + 4, sizeof(storage_header_t) - 4, 0 );
      header.crc32   = Crc_Crc32( (uint8_t*)StorageFlash_Data.headers[i] + sizeof(storage_header_t), size, crc );

      // write migrated header with adapted counter and checksum
      Target_NvMemWrite( active_page, active_page_addr, (void*)&header, sizeof(storage_header_t) );

      // copy data to active page
      Target_NvMemWrite( active_page, active_page_addr + sizeof(storage_header_t),
                         (void*)((uint8_t*)StorageFlash_Data.headers[i] + sizeof(storage_header_t)), size );

      // update storage header
      StorageFlash_Data.headers[i] = (storage_header_t*)(&TARGET_NV_MEM->pages[StorageFlash_Data.active_page][StorageFlash_Data.pages_info[StorageFlash_Data.active_page].address]);
      // update page info
      StorageFlash_Data.pages_info[StorageFlash_Data.active_page].address += (sizeof(storage_header_t) + StorageFlash_GetSizeRoundedToFlashBlock(size));
    }
  }
  return TRUE;
}


error_code_t StorageFlash_RenewPage( uint8_t page )
{
#if( 32 == NV_MEM_DATA_ALIGNMENT )
  uint32_t buffer[NV_MEM_PAGE_INFO_ADDR/4];
#elif( 64 == NV_MEM_DATA_ALIGNMENT )
  uint64_t buffer[NV_MEM_PAGE_INFO_ADDR/8];
#endif
  uint16_t buffer_index = 0;
  storage_id_t i;
  int16_t addresses[NR_OF_STORAGES];
  bool_t  address_is_valid[NR_OF_STORAGES] = {FALSE};
  error_code_t error;

  // check page
  if( NV_MEM_TOTAL_PAGES <= page )
    return INDEX_OUT_OF_BOUNDARY;

  // copy page's valid initialized data to buffer
  for( i = 0; i < NR_OF_STORAGES; i ++ )
  {
    if( StorageFlash_HeaderGetPage(StorageFlash_Data.headers[i]) == page )
    {
      memcpy( &buffer[buffer_index], StorageFlash_Data.headers[i], sizeof(storage_header_t) + StorageFlash_Data.headers[i]->size );
      address_is_valid[i] = TRUE;
#if( 32 == NV_MEM_DATA_ALIGNMENT )
      addresses[i] = buffer_index * 4;
      buffer_index += ((sizeof(storage_header_t) + StorageFlash_Data.headers[i]->size) / 4);
#elif( 64 == NV_MEM_DATA_ALIGNMENT )
      addresses[i] = buffer_index * 8;
      buffer_index += ((sizeof(storage_header_t) + StorageFlash_Data.headers[i]->size) / 8);
#endif
    }
  }

  // erase page
  StorageFlash_ErasePage( page );

  // if there was valid data, write back to page
  if( 0 < buffer_index )
  {
#if( 32 == NV_MEM_DATA_ALIGNMENT )
    error = Target_NvMemWrite( page, 0, buffer, buffer_index * 4 );
#elif( 64 == NV_MEM_DATA_ALIGNMENT )
    error = Target_NvMemWrite( page, 0, buffer, buffer_index * 8 );
#endif
    if( OK != error )
      return error;
#if( 32 == NV_MEM_DATA_ALIGNMENT )
    StorageFlash_Data.pages_info[page].address = buffer_index * 4;
#elif( 64 == NV_MEM_DATA_ALIGNMENT )
    StorageFlash_Data.pages_info[page].address = buffer_index * 8;
#endif

    // set headers according to new new addresses
    for( i = 0; i < NR_OF_STORAGES; i ++ )
    {
      if( address_is_valid[i] )
        StorageFlash_Data.headers[i] = (storage_header_t*)(&TARGET_NV_MEM->pages[page][addresses[i]]);
    }
  }

  return OK;
}


void StorageFlash_PrepareAffectedPages( uint16_t size )
{
  // calculate next page
  uint8_t next_page = StorageFlash_Data.active_page + 1;
  if( NV_MEM_TOTAL_PAGES <= next_page )
    next_page = 0;

  // active page can only be marked as corrupted during initialization due to faulty data
  if( PAGE_IS_CORRUPTED == StorageFlash_Data.pages_info[StorageFlash_Data.active_page].status )
  {
    // repair corrupted page
    StorageFlash_RenewPage( StorageFlash_Data.active_page );
  }

  // ensure next page is empty and valid
  if( (0 != StorageFlash_Data.pages_info[next_page].address) || (PAGE_IS_VALID != StorageFlash_Data.pages_info[next_page].status) )
  {
    // next page is not empty or valid, try migration to active page
    if( ! StorageFlash_MigrateToActivePage(next_page) )
    {
      // TODO Fancy algorithm to resolve this particular issue
      // Currently data gets lost, however if control flow really comes to this
      // point, some weirdness was already going on during previous runtime.
/*
      // clean up next page, keep active valid data --> similar to resolving corrupted page
      Storage_RenewPage( next_page );
      // set next page active
      uint8_t prev_page = Storage_Data.active_page;
      Storage_Data.active_page = next_page;
      // migrate previously active page
      Storage_MigrateToActivePage( prev_page );
      // erase previously active page
      Storage_ErasePage( prev_page );
      return;
*/
    }
    StorageFlash_ErasePage( next_page );
  }

  // check size against remaining space and conditionally set next page active
  if( NV_MEM_PAGE_INFO_ADDR < (StorageFlash_Data.pages_info[StorageFlash_Data.active_page].address + sizeof(storage_header_t) + size) )
    StorageFlash_Data.active_page = next_page;
}


/**
 * See Storage_Write() for further information.
 */
error_code_t StorageFlash_Write( storage_id_t storage_id, void *data, int16_t size )
{
  storage_header_t header;
  uint32_t crc;
  error_code_t error_code;

  // perform some plausibility checks
  if( ! StorageFlash_Data.is_initialized )
    return STORAGE_NOT_INITIALIZED;
  if( NR_OF_STORAGES <= storage_id )
    return INDEX_OUT_OF_BOUNDARY;
  if( NV_MEM_PAGE_INFO_ADDR < (sizeof(storage_header_t) + size) )
    return STORAGE_DATA_TOO_BIG;
  if( PAGE_UNINITIALIZED == StorageFlash_Data.pages_info[StorageFlash_Data.active_page].status )
  {
    // due to previous initialization check, control flow must not end here
    return UNKNOWN_ERROR;
  }

  StorageFlash_PrepareAffectedPages( size );

  // compare data before issuing flash writing
  if( (NULL != StorageFlash_Data.headers[storage_id]) && (0 == memcmp( StorageFlash_Data.headers[storage_id] + 1, data, size )) )
    return OK;

  // prepare and write header
  header.size  = size;
  header.index = storage_id;
  if( NULL == StorageFlash_Data.headers[storage_id] )
    header.counter = 0;
  else
    header.counter = StorageFlash_Data.headers[storage_id]->counter + 1;
  crc = Crc_Crc32((uint8_t*)(&header) + 4, sizeof(storage_header_t) - 4, 0);
  header.crc32 = Crc_Crc32(data, size, crc);
#if( 32 == NV_MEM_DATA_ALIGNMENT )
  error_code = Target_NvMemWrite( StorageFlash_Data.active_page, StorageFlash_Data.pages_info[StorageFlash_Data.active_page].address,
                                  (uint32_t*)(&header), sizeof(storage_header_t) );
#elif( 64 == NV_MEM_DATA_ALIGNMENT )
  error_code = Target_NvMemWrite( StorageFlash_Data.active_page, StorageFlash_Data.pages_info[StorageFlash_Data.active_page].address,
                                  (uint64_t*)(&header), sizeof(storage_header_t) );
#endif
  if (OK != error_code)
    return error_code;

  // write data
  error_code = Target_NvMemWrite( StorageFlash_Data.active_page,
                                  StorageFlash_Data.pages_info[StorageFlash_Data.active_page].address + sizeof(storage_header_t), data, size );
  if (OK != error_code)
    return error_code;

  // set new addresses
  StorageFlash_Data.headers[storage_id] = (storage_header_t*)(&TARGET_NV_MEM->pages[StorageFlash_Data.active_page][StorageFlash_Data.pages_info[StorageFlash_Data.active_page].address]);
  StorageFlash_Data.pages_info[StorageFlash_Data.active_page].address += (sizeof(storage_header_t) + StorageFlash_GetSizeRoundedToFlashBlock(size));

  return OK;
}


/**
 * See Storage_Read() for further information.
 */
int32_t StorageFlash_Read( storage_id_t storage_id, void *data, int16_t size )
{
  // perform some plausibility checks
  if( ! StorageFlash_Data.is_initialized )
    return (int32_t)STORAGE_NOT_INITIALIZED;
  if( NR_OF_STORAGES <= storage_id )
    return (int32_t)INDEX_OUT_OF_BOUNDARY;
  if( NULL == StorageFlash_Data.headers[storage_id] )
    return (int32_t)STORAGE_DATA_UNAVAILABLE;

  storage_header_t * header = StorageFlash_Data.headers[storage_id];
  if( header->size > size )
    return (int32_t)BUFFER_TOO_SMALL;
  if( header->crc32 != Crc_Crc32(((uint8_t*)header) + 4, sizeof(storage_header_t) - 4 + header->size, 0) )
    return (int32_t)STORAGE_CRC_MISMATCH;

  memcpy( data, header + 1, header->size );
  return header->size;
}


/**
 * @} end of STORAGE_FLASH
 */
