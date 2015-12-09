#    This program and the accompanying materials
#    are licensed and made available under the terms and conditions of the BSD License
#    which accompanies this distribution. The full text of the license may be found at
#    http://opensource.org/licenses/bsd-license.php
#
#    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

[Defines]
  PLATFORM_NAME                  = AndroidFastbootApp
  PLATFORM_GUID                  = 5C4690F0-4B99-46B8-9464-D5F6D9FF9DDD
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/AndroidFastboot
  SUPPORTED_ARCHITECTURES        = ARM|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses.common]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLibOptionalDevicePathProtocol.inf
  DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf  
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  NetLib|MdeModulePkg/Library/DxeNetLib/DxeNetLib.inf

  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf

  ArmGicLib|ArmPkg/Drivers/ArmGic/ArmGicLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf

[LibraryClasses.ARM]
  #
  # It is not possible to prevent the ARM compiler for generic intrinsic functions.
  # This library provides the instrinsic functions generate by a given compiler.
  # [LibraryClasses.ARM] and NULL mean link this library into all ARM images.
  #
  NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf

  # Add support for GCC stack protector
  NULL|MdePkg/Library/BaseStackCheckLib/BaseStackCheckLib.inf

  ArmLib|ArmPkg/Library/ArmLib/ArmV7/ArmV7Lib.inf
  ArmSmcLib|ArmPkg/Library/ArmSmcLib/ArmSmcLib.inf
  BdsLib|ArmPkg/Library/BdsLib/BdsLib.inf

[LibraryClasses.AARCH64]
  NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf

  ArmLib|ArmPkg/Library/ArmLib/AArch64/AArch64Lib.inf
  ArmSmcLib|ArmPkg/Library/ArmSmcLib/ArmSmcLib.inf
  BdsLib|ArmPkg/Library/BdsLib/BdsLib.inf

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|16000

[Components]
  EmbeddedPkg/Application/AndroidFastboot/AndroidFastbootApp.inf
