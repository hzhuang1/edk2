/** @file

  Copyright (c) 2013-2014, ARM Ltd. All rights reserved.<BR>
  Copyright (c) 2017, Linaro. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/BaseMemoryLib.h>
#include <Library/BdsLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePathFromText.h>

#include "AndroidBootApp.h"

#define IS_DEVICE_PATH_NODE(node,type,subtype) (((node)->Type == (type)) && ((node)->SubType == (subtype)))

#if 0
STATIC
BOOLEAN
CompareDevicePath (
  IN EFI_DEVICE_PATH    *DevicePath1,
  IN EFI_DEVICE_PATH    *DevicePath2
  )
{
  UINTN     Size1, Size2;

  Size1 = GetDevicePathSize (DevicePath1);
  Size2 = GetDevicePathSize (DevicePath2);
  if (Size1 != Size2)
    return FALSE;
  if (Size1 == 0)
    return FALSE;
  if (CompareMem (DevicePath1, DevicePath2, Size1) != 0) {
    return FALSE;
  }

  return TRUE;
}

STATIC
EFI_STATUS
BdsLocateBootOption (
  IN EFI_DEVICE_PATH    *DevicePath,
  OUT BDS_LOAD_OPTION  **BdsLoadOption
  )
{
  UINTN             Index;
  EFI_STATUS        Status;
  BDS_LOAD_OPTION  *LoadOption;

  for (Index = 0; ; Index++) {
    Status = BootOptionFromLoadOptionIndex (Index, &LoadOption);
    if (EFI_ERROR (Status))
      return Status;
    if (CompareDevicePath (DevicePath, LoadOption->FilePathList) == FALSE)
      continue;
    *BdsLoadOption = LoadOption;
    return EFI_SUCCESS;
  }
  return Status;
}

STATIC
EFI_STATUS
LoadAndroidBootImg (
  IN UINTN                    BufferSize,
  IN VOID                    *Buffer,
  IN BDS_LOAD_OPTION         *BdsLoadOption,
  OUT EFI_PHYSICAL_ADDRESS   *Image,
  OUT UINTN                  *ImageSize
  )
{
  EFI_STATUS                  Status;
  EFI_PHYSICAL_ADDRESS        KernelBase, RamdiskBase, FdtBase;
  UINTN                       KernelSize;
  ANDROID_BOOTIMG_HEADER     *Header;
  CHAR16                      KernelArgs[BOOTIMG_KERNEL_ARGS_SIZE];
  CHAR16                      InitrdArgs[64];
  UINTN                       VariableSize;
  CHAR16                      SerialNoArgs[40], DataUnicode[17];

  Header = (ANDROID_BOOTIMG_HEADER *) Buffer;

  if (AsciiStrnCmp (Header->BootMagic, BOOT_MAGIC, BOOT_MAGIC_LENGTH) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (Header->KernelSize == 0) {
    return EFI_NOT_FOUND;
  }

  ASSERT (IS_POWER_OF_2 (Header->PageSize));

  KernelBase = Header->KernelAddress;
  Status = gBS->AllocatePages (AllocateAddress, EfiBootServicesCode,
                               EFI_SIZE_TO_PAGES (Header->KernelSize), (VOID *)&KernelBase);
  ASSERT_EFI_ERROR (Status);
  CopyMem ((VOID *)KernelBase,
           (CONST VOID *)((UINTN)Buffer + Header->PageSize),
           Header->KernelSize);

  RamdiskBase = Header->RamdiskAddress;
  if (Header->RamdiskSize != 0) {
    Status = gBS->AllocatePages (AllocateAnyPages, EfiBootServicesCode,
                                 EFI_SIZE_TO_PAGES (Header->RamdiskSize), (VOID *)&RamdiskBase);
    ASSERT_EFI_ERROR (Status);
    CopyMem ((VOID *)RamdiskBase,
             (VOID *)((UINTN)Buffer + Header->PageSize + ALIGN_VALUE (Header->KernelSize, Header->PageSize)),
             Header->RamdiskSize
            );
    if (RamdiskBase != Header->RamdiskAddress)
      Header->RamdiskAddress = RamdiskBase;
  }
  /* Install Fdt */
  KernelSize = *(UINT32 *)(KernelBase + KERNEL_IMAGE_STEXT_OFFSET) +
               *(UINT32 *)(KernelBase + KERNEL_IMAGE_RAW_SIZE_OFFSET);
  ASSERT (KernelSize < Header->KernelSize);

  /* FDT is at the end of kernel image */
  FdtBase = KernelBase + KernelSize;
  Status = gBS->InstallConfigurationTable (
                  &gFdtTableGuid,
                  (VOID *)FdtBase
                  );
  ASSERT_EFI_ERROR (Status);

  /* update kernel args */
  AsciiStrToUnicodeStr (Header->KernelArgs, KernelArgs);
  if (StrnCmp (KernelArgs, BdsLoadOption->OptionalData,
               BOOTIMG_KERNEL_ARGS_SIZE) != 0) {
    ASSERT (BdsLoadOption->OptionalData != NULL);
    ASSERT (StrSize (KernelArgs) <= BOOTIMG_KERNEL_ARGS_SIZE);

    UnicodeSPrint (InitrdArgs, 64 * sizeof(CHAR16), L" initrd=0x%x,0x%x",
                   Header->RamdiskAddress, Header->RamdiskSize);
    StrCat (KernelArgs, InitrdArgs);
    VariableSize = 17 * sizeof (CHAR16);
    Status = gRT->GetVariable (
                    (CHAR16 *)L"SerialNo",
                    &gHiKeyVariableGuid,
                    NULL,
                    &VariableSize,
                    &DataUnicode
                    );
    if (EFI_ERROR (Status)) {
      goto out;
    }
    DataUnicode[(VariableSize / sizeof(CHAR16)) - 1] = '\0';
    ZeroMem (SerialNoArgs, 40 * sizeof (CHAR16));
    UnicodeSPrint (SerialNoArgs, 40 * sizeof(CHAR16), L" androidboot.serialno=%s", DataUnicode);
    StrCat (KernelArgs, SerialNoArgs);
    ASSERT (StrSize (KernelArgs) <= BOOTIMG_KERNEL_ARGS_SIZE);
    if (gArgs != NULL) {
      CopyMem ((VOID *)gArgs,
               (VOID *)KernelArgs,
               StrSize (KernelArgs)
              );
    }
  }

  *Image = KernelBase;
  *ImageSize = Header->KernelSize;
  return EFI_SUCCESS;
out:
  return Status;
}
#endif

EFI_STATUS
EFIAPI
AndroidBootAppEntryPoint (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_STATUS                          Status;
  CHAR16                              *BootPathStr;
  EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL  *EfiDevicePathFromTextProtocol;
  EFI_DEVICE_PATH                     *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL            *Node, *NextNode;
  EFI_BLOCK_IO_PROTOCOL               *BlockIo;
  HARDDRIVE_DEVICE_PATH               *PartitionPath;
  UINT32                              MediaId, BlockSize;
  VOID                                *Buffer;
  EFI_HANDLE                          Handle;

  BootPathStr = (CHAR16 *)PcdGetPtr (PcdAndroidBootDevicePath);
  ASSERT (BootPathStr != NULL);
DEBUG ((DEBUG_ERROR, "#%a, %d, BootPathStr:%s\n", __func__, __LINE__, BootPathStr));
  Status = gBS->LocateProtocol (&gEfiDevicePathFromTextProtocolGuid, NULL, (VOID **)&EfiDevicePathFromTextProtocol);
  ASSERT_EFI_ERROR(Status);
  DevicePath = (EFI_DEVICE_PATH *)EfiDevicePathFromTextProtocol->ConvertTextToDevicePath (BootPathStr);
  ASSERT (DevicePath != NULL);

DEBUG ((DEBUG_ERROR, "#%a, %d\n", __func__, __LINE__));

  /* Find DevicePath node of Partition */
  NextNode = DevicePath;
  while (1) {
DEBUG ((DEBUG_ERROR, "#%a, %d, Type:0x%x, SubType:0x%x\n", __func__, __LINE__, NextNode->Type, NextNode->SubType));
    Node = NextNode;
    if (IS_DEVICE_PATH_NODE (Node, MEDIA_DEVICE_PATH, MEDIA_HARDDRIVE_DP)) {
      PartitionPath = (HARDDRIVE_DEVICE_PATH *)Node;
DEBUG ((DEBUG_ERROR, "#%a, %d, Type:0x%x, SubType:0x%x\n", __func__, __LINE__, Node->Type, Node->SubType));
DEBUG ((DEBUG_ERROR, "#%a, %d, PartitionNumber:0x%x, PartitionSize:0x%x\n", __func__, __LINE__, PartitionPath->PartitionNumber, PartitionPath->PartitionSize));
      break;
    }
    NextNode = NextDevicePathNode (Node);
  }
#if 0
DEBUG ((DEBUG_ERROR, "#%a, %d\n", __func__, __LINE__));
  PartitionPath = (HARDDRIVE_DEVICE_PATH *)Node;
#endif

  Status = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &DevicePath, &Handle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlockIo,
                  gImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  ASSERT_EFI_ERROR (Status);

  MediaId = BlockIo->Media->MediaId;
  BlockSize = BlockIo->Media->BlockSize;
  /* Both PartitionStart and PartitionSize are counted as block size. */
  Buffer = AllocatePages (EFI_SIZE_TO_PAGES (PartitionPath->PartitionSize));
  if (Buffer == NULL) {
DEBUG ((DEBUG_ERROR, "#%a, %d\n", __func__, __LINE__));
    return EFI_BUFFER_TOO_SMALL;
  }

  /* Load header of boot.img */
  Status = BlockIo->ReadBlocks (
                      BlockIo,
                      MediaId,
                      PartitionPath->PartitionStart / BlockSize,
                      PartitionPath->PartitionSize,
                      Buffer
                      );
  if (EFI_ERROR (Status)) {
DEBUG ((DEBUG_ERROR, "#%a, %d\n", __func__, __LINE__));
    DEBUG ((EFI_D_ERROR, "Failed to read blocks: %r\n", Status));
    return Status;
  }

  Status = BootAndroidBootImg (PartitionPath->PartitionSize, Buffer);
DEBUG ((DEBUG_ERROR, "#%a, %d, Status:%r\n", __func__, __LINE__, Status));
ASSERT(0);
  //Status = LoadAndroidBootImg (PartitionPath->PartitionSize, Buffer, BdsLoadOption, Image, ImageSize);
  FreePages (Buffer, EFI_SIZE_TO_PAGES (PartitionPath->PartitionSize));
  return Status;
}
