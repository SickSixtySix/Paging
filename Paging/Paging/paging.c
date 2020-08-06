#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Structure of a virtual address
typedef struct vaddr {
	uint64_t	offset	: 12;	// Offset within a page
	uint64_t	pte		: 9;	// Level 4 table entry
	uint64_t	pde		: 9;	// Level 3 table entry
	uint64_t	pdpte	: 9;	// Level 2 table entry
	uint64_t	pml4e	: 9;	// Level 1 table entry
	uint64_t			: 16;	// ...
} vaddr_t;

// Structure of a page descriptor
typedef struct pdesc {
	uint64_t	p		: 1;	// Presence flag
	uint64_t			: 1;	// Write flag
	uint64_t			: 1;	// Unprivileged flag
	uint64_t			: 4;	// ...
	uint64_t			: 1;	// Last level flag
	uint64_t			: 4;	// ...
	uint64_t	paddr	: 40;	// Physical address
	uint64_t			: 11;	// ...
	uint64_t			: 1;	// Execution flag
} pdesc_t;

// Structure of a memory cell
typedef struct mcell {
	uint64_t	paddr;	// Physical address of 8-byte cell
	pdesc_t		pdesc;	// Cells within page tables store page descriptors
} mcell_t;

// Structure of a memory
typedef struct pmemo {
	uint64_t	pbase;	// Level 1 table physical address
	mcell_t		*mcell;	// Array of 8-byte memory cells
	uint64_t	count;	// Count of 8-byte memory cells
} pmemo_t;

int main() {
	return 0;
}