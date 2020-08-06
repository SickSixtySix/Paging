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
	uint64_t	ps		: 1;	// Last level flag
	uint64_t			: 4;	// ...
	uint64_t	paddr	: 40;	// Physical address
	uint64_t			: 11;	// ...
	uint64_t			: 1;	// Execution flag
} pdesc_t;

// Size of a page descriptor in bytes
#define PDESC_SIZE sizeof(pdesc)

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

/* Gets page descriptor by a physical address of the page table and index of the entry */
uint64_t get_pdesc(const pmemo_t *const pmemo, const uint64_t base, const uint64_t index, pdesc_t *const pdesc) {
	// Physical address of the entry
	const uint64_t paddr = base + index * PDESC_SIZE;

	// Find a corresponding page descriptor
	uint64_t i;
	for (i = 0; i < pmemo->count; i++) {
		const mcell_t* const mcell = &pmemo->mcell[i];
		if (mcell->paddr == paddr) {
			memcpy(&pdesc, &mcell->pdesc, PDESC_SIZE);
			return 0;
		}
	}

	return 1;
}

/* Converts a virtual address to a physical address */
uint64_t get_paddr(const pmemo_t *const pmemo, const uint64_t vaddr, uint64_t *const paddr) {
	// Convert numerical reprensentation of the virtual address into the structured representation
	const vaddr_t* const addr = (vaddr_t*)&vaddr;

	// Level 1
	pdesc_t pml4e;
	if (get_pdesc(pmemo, pmemo->pbase, addr->pml4e, &pml4e) || !pml4e.p) {
		return 1;
	}

	// Level 2 (with PS flag for 1 GB page)
	pdesc_t pdpte;
	if (get_pdesc(pmemo, pml4e.paddr, addr->pdpte, &pdpte) || !pdpte.p) {
		return 1;
	}
	else if (pdpte.ps) {
		*paddr = pdpte.paddr + addr->offset;
		return 0;
	}

	// Level 3 (with PS flag for 2 MB page)
	pdesc_t pde;
	if (get_pdesc(pmemo, pdpte.paddr, addr->pde, &pde) || !pde.p) {
		return 1;
	}
	else if (pde.ps) {
		*paddr = pde.paddr + addr->offset;
		return 0;
	}

	// Level 4
	pdesc_t pte;
	if (get_pdesc(pmemo, pde.paddr, addr->pte, &pte) || !pte.p) {
		return 1;
	}

	*paddr = pte.paddr + addr->offset;
	return 0;
}

int main() {
	return 0;
}