.text
.global arch_ctx_restore

.equ OFFSET_SP,   0x00
.equ OFFSET_PC,   0x04
.equ OFFSET_EBX,  0x08
.equ OFFSET_ESI,  0x0C
.equ OFFSET_EDI,  0x10
.equ OFFSET_EBP,  0x14

.equ OFFSET_IPL,  0x18
.equ OFFSET_TLS,  0x18

.macro CONTEXT_RESTORE_ARCH_CORE ctx:req pc:req
	movl OFFSET_SP(\ctx),%esp	# ctx->sp -> %esp
	movl OFFSET_PC(\ctx),\pc	# ctx->pc -> \pc
	movl OFFSET_EBX(\ctx),%ebx	# ctx->ebx -> %ebx
	movl OFFSET_ESI(\ctx),%esi	# ctx->esi -> %esi
	movl OFFSET_EDI(\ctx),%edi	# ctx->edi -> %edi
	movl OFFSET_EBP(\ctx),%ebp	# ctx->ebp -> %ebp
.endm


## Restore saved CPU context
#
# Restore CPU context from context_t variable
# pointed by the 1st argument. Returns 0 in EAX.
#
arch_ctx_restore:
	movl 4(%esp),%eax	# address of the context variable to restore context from

		# restore registers from given structure
	CONTEXT_RESTORE_ARCH_CORE %eax %edx

	movl %edx,0(%esp)	# put saved pc on stack
	xorl %eax,%eax		# context_restore returns 0
	ret
