//
// d_copy.s
// x86 assembly-language screen copying code.
//

#include "asm_i386.h"
#include "quakeasm.h"
#include "asm_draw.h"

	.data

LCopyWidth:		.long	0
LBlockSrcStep:	.long	0
LBlockDestStep:	.long	0
LSrcDelta:		.long	0
LDestDelta:		.long	0

#define bufptr	4+16

	.text


#define srcptr			4+16
#define destptr			8+16
#define width			12+16
#define height			16+16
#define srcrowbytes		20+16
#define destrowbytes	24+16

.globl C(VGA_UpdateLinearScreen)
C(VGA_UpdateLinearScreen):
	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi
	pushl	%esi				// preserve register variables
	pushl	%ebx

	cld
	movl	srcptr(%esp),%esi
	movl	destptr(%esp),%edi
	movl	width(%esp),%ebx
	movl	srcrowbytes(%esp),%eax
	subl	%ebx,%eax
	movl	destrowbytes(%esp),%edx
	subl	%ebx,%edx
	shrl	$2,%ebx
	movl	height(%esp),%ebp
LLRowLoop:
	movl	%ebx,%ecx
	rep/movsl	(%esi),(%edi)
	addl	%eax,%esi
	addl	%edx,%edi
	decl	%ebp
	jnz		LLRowLoop

	popl	%ebx				// restore register variables
	popl	%esi
	popl	%edi
	popl	%ebp				// restore the caller's stack frame

	ret

