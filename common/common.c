/****************************
* 2008.3     ITL		Implement  NewBluePill Project on x86_64  
* 2011.1     Miao Yu     	Reorganize it for Vis hypervisor on x86 and x86_64(not finished).
* 
*****************************/

#include "common.h"
#include <vis/vcpu.h>
#include <vis/config.h>
#include <asm/system.h>

/**
 * effects:Raise the interruption level to dispatch level, then
 * install VM Root hypervisor by call <CallbackProc>
 */
NTSTATUS NTAPI CmDeliverToProcessor (
  CCHAR cProcessorNumber,
  PCALLBACK_PROC CallbackProc,
  PVOID CallbackParam,
  PNTSTATUS pCallbackStatus
)
{ 
  NTSTATUS CallbackStatus;
  KIRQL OldIrql;

  if (!CallbackProc)
    return STATUS_INVALID_PARAMETER;

  if (pCallbackStatus)
    *pCallbackStatus = STATUS_UNSUCCESSFUL;

  KeSetSystemAffinityThread ((KAFFINITY) (1 << cProcessorNumber));

  OldIrql = KeRaiseIrqlToDpcLevel ();
  CallbackStatus = CallbackProc (CallbackParam);

  KeLowerIrql (OldIrql);

  KeRevertToUserAffinityThread ();

  // save the status of the callback which has run on the current core
  if (pCallbackStatus)
    *pCallbackStatus = CallbackStatus;

  return STATUS_SUCCESS;
}

NTSTATUS NTAPI CmInitializeSegmentSelector (
    SEGMENT_SELECTOR *pSegmentSelector,
    USHORT Selector,
    PUCHAR GdtBase
)
{
    PSEGMENT_DESCRIPTOR SegDesc;

    if (!pSegmentSelector)
        return STATUS_INVALID_PARAMETER;

    if (Selector & 0x4) 
    {
		KdPrint(("Helloworld:CmInitializeSegmentSelector(): Given selector (0x%X) points to LDT\n", Selector));
        return STATUS_INVALID_PARAMETER;
    }

    SegDesc = (PSEGMENT_DESCRIPTOR) ((PUCHAR) GdtBase + (Selector & ~0x7));

    pSegmentSelector->sel = Selector;
    pSegmentSelector->base = SegDesc->base0 | SegDesc->base1 << 16 | SegDesc->base2 << 24;
    pSegmentSelector->limit = SegDesc->limit0 | (SegDesc->limit1attr1 & 0xf) << 16;
    pSegmentSelector->attributes.UCHARs = SegDesc->attr0 | (SegDesc->limit1attr1 & 0xf0) << 4;

    //if (!(SegDesc->attr0 & LA_STANDARD)) 
    //{
    //    ULONG64 tmp;
    //    // this is a TSS or callgate etc, save the base high part
    //    tmp = (*(PULONG64) ((PUCHAR) SegDesc + 8));
    //    SegmentSelector->base = (SegmentSelector->base & 0xffffffff) | (tmp << 32);
    //}

    if (pSegmentSelector->attributes.fields.g) 
    {
        // 4096-bit granularity is enabled for this segment, scale the limit
        pSegmentSelector->limit = (pSegmentSelector->limit << 12) + 0xfff;
    }

    return STATUS_SUCCESS;
}

// generate binary code
NTSTATUS NTAPI CmGenerateMovReg (
  PUCHAR pCode,
  PULONG pGeneratedCodeLength,
  ULONG Register,
  ULONG Value
)
{ //Finished
    ULONG uCodeLength;

    if (!pCode || !pGeneratedCodeLength)
        return STATUS_INVALID_PARAMETER;

    switch (Register & ~REG_MASK) 
    {
    case REG_GP:
        pCode[0] = 0xb8 | (UCHAR) (Register & REG_MASK);
        memcpy (&pCode[1], &Value, 4);
        uCodeLength = 5;
        break;

    case REG_GP_ADDITIONAL:
        pCode[0] = 0xb8 | (UCHAR) (Register & REG_MASK);
        memcpy (&pCode[1], &Value, 4);
        uCodeLength = 5;
        break;

    case REG_CONTROL:
        uCodeLength = *pGeneratedCodeLength;
        CmGenerateMovReg (pCode, pGeneratedCodeLength, REG_RAX, Value);
        // calc the size of the "mov rax, value"
        uCodeLength = *pGeneratedCodeLength - uCodeLength;
        pCode += uCodeLength;

        // mov crX, rax

        pCode[0] = 0x0f;
        pCode[1] = 0x22;
        pCode[2] = 0xc0 | (UCHAR) ((Register & REG_MASK) << 3);

        // *pGeneratedCodeLength has already been adjusted to the length of the "mov rax"
        uCodeLength = 3;
    }

    if (pGeneratedCodeLength)
        *pGeneratedCodeLength += uCodeLength;

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI CmGeneratePushReg (
    PUCHAR pCode,
    PULONG pGeneratedCodeLength,
    ULONG Register
)
{
    if (!pCode || !pGeneratedCodeLength)
        return STATUS_INVALID_PARAMETER;

    if ((Register & ~REG_MASK) != REG_GP)
        return STATUS_NOT_SUPPORTED;

    pCode[0] = 0x50 | (UCHAR) (Register & REG_MASK);
    *pGeneratedCodeLength += 1;

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI CmGenerateIretd (
    PUCHAR pCode,
    PULONG pGeneratedCodeLength
)
{
    if (!pCode || !pGeneratedCodeLength)
        return STATUS_INVALID_PARAMETER;

    pCode[0] = 0xcf;
    *pGeneratedCodeLength += 1;

    return STATUS_SUCCESS;
};