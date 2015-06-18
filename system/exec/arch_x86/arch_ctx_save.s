.text
.global arch_ctx_save

.equ OFFSET_SP,   0x00
.equ OFFSET_PC,   0x04
.equ OFFSET_EBX,  0x08
.equ OFFSET_ESI,  0x0C
.equ OFFSET_EDI,  0x10
.equ OFFSET_EBP,  0x14

.equ OFFSET_IPL,  0x18
.equ OFFSET_TLS,  0x18

.macro CONTEXT_SAVE_ARCH_CORE ctx:req pc:req
	movl %esp,OFFSET_SP(\ctx)	# %esp -> ctx->sp
	movl \pc,OFFSET_PC(\ctx)	# %eip -> ctx->pc
	movl %ebx,OFFSET_EBX(\ctx)	# %ebx -> ctx->ebx
	movl %esi,OFFSET_ESI(\ctx)	# %esi -> ctx->esi
	movl %edi,OFFSET_EDI(\ctx)	# %edi -> ctx->edi
	movl %ebp,OFFSET_EBP(\ctx)	# %ebp -> ctx->ebp
.endm



## Save current CPU context
#
# Save CPU context to the context variable
# pointed by the 1st argument. Returns 1 in EAX.
#
arch_ctx_save:
	movl 0(%esp),%eax	# save pc value into eax
	movl 4(%esp),%edx	# address of the context variable to save context to

		# save registers to given structure
	CONTEXT_SAVE_ARCH_CORE %edx %eax

	xorl %eax,%eax		# context_save returns 1
	incl %eax
	ret
