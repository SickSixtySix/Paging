#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Virtual address data type
typedef union vaddr {
	uint64_t	val;	// Unstructured value
	struct
	{
		uint64_t	offset	: 12;	// Offset within a page
		uint64_t	pte		: 9;	// Level 4 table entry
		uint64_t	pde		: 9;	// Level 3 table entry
		uint64_t	pdpte	: 9;	// Level 2 table entry
		uint64_t	pml4e	: 9;	// Level 1 table entry
		uint64_t			: 16;	// ...
	}			stc;	// Structured value
} vaddr_t;

// Page descriptor data type
typedef union pdesc {
	uint64_t	val;	// Unstructured value
	struct
	{
		uint64_t	p		: 1;	// Presence flag
		uint64_t			: 1;	// Write flag
		uint64_t			: 1;	// Unprivileged flag
		uint64_t			: 4;	// ...
		uint64_t	ps		: 1;	// Last level flag
		uint64_t			: 4;	// ...
		uint64_t	paddr	: 40;	// Physical address (without 12 least bits)
		uint64_t			: 11;	// ...
		uint64_t			: 1;	// Execution flag
	}			stc;	// Structured value
} pdesc_t;

// Converts a page descriptor to a page physical address
#define PDESC_2_PADDR(X) (X.stc.paddr << 12)

// Size of a page descriptor in bytes
#define PDESC_SIZE sizeof(pdesc_t)

// Structure of a memory cell
typedef struct mcell {
	uint64_t	paddr;	// Physical address of 8-byte cell
	pdesc_t		pdesc;	// Cells within page tables store page descriptors
} mcell_t;

// Size of a memory cell in bytes
#define MCELL_SIZE sizeof(mcell_t)

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
			pdesc->val = mcell->pdesc.val;
			return 0;
		}
	}

	return 1;
}

/* Converts a virtual address to a physical address */
uint64_t get_paddr(const pmemo_t *const pmemo, const vaddr_t vaddr, uint64_t *const paddr) {
	// Level 1
	pdesc_t pml4e;
	if (get_pdesc(pmemo, pmemo->pbase, vaddr.stc.pml4e, &pml4e) || !pml4e.stc.p) {
		return 1;
	}

	// Level 2 (with PS flag for 1 GB page)
	pdesc_t pdpte;
	if (get_pdesc(pmemo, PDESC_2_PADDR(pml4e), vaddr.stc.pdpte, &pdpte) || !pdpte.stc.p) {
		return 1;
	}
	/*else if (pdpte.stc.ps) {
		*paddr = PDESC_2_PADDR(pdpte) + vaddr.stc.offset;
		return 0;
	}*/

	// Level 3 (with PS flag for 2 MB page)
	pdesc_t pde;
	if (get_pdesc(pmemo, PDESC_2_PADDR(pdpte), vaddr.stc.pde, &pde) || !pde.stc.p) {
		return 1;
	}
	/*else if (pde.stc.ps) {
		*paddr = PDESC_2_PADDR(pde) + vaddr.stc.offset;
		return 0;
	}*/

	// Level 4
	pdesc_t pte;
	if (get_pdesc(pmemo, PDESC_2_PADDR(pde), vaddr.stc.pte, &pte) || !pte.stc.p) {
		return 1;
	}

	*paddr = PDESC_2_PADDR(pte) + vaddr.stc.offset;
	return 0;
}

int main() {
	// Open the input file
	FILE *pin = fopen("dataset.txt", "r");

	if (!pin) {
		printf("dataset.txt cannot be found\n");
		getc(stdin);
		return -1;
	}

	// Read the memory cells number, the queries number and the physical address of a root page table
	uint64_t m, q, r;
	fscanf(pin, "%llu %llu %llu", &m, &q, &r);

	// Read the memory state
	pmemo_t pmemo = {
		r, (mcell_t*)malloc(MCELL_SIZE * (size_t)m), m
	};

	for (uint64_t i = 0; i < pmemo.count; i++) {
		mcell_t *const mcell = &pmemo.mcell[i];
		fscanf(pin, "%llu %llu", &mcell->paddr, &mcell->pdesc.val);

		if (!mcell->pdesc.stc.p) {
			--pmemo.count;
			--i;
		}
	}

	// Open the output file
	FILE *pout = fopen("results.txt", "w");

	// Perform the queries and flush results every 100 queries
	for (uint64_t i = 0; i < q; i++) {
		vaddr_t vaddr;
		fscanf(pin, "%llu", &vaddr.val);

		uint64_t paddr;
		if (get_paddr(&pmemo, vaddr, &paddr)) {
			fprintf(pout, "fault\n");
		}
		else {
			fprintf(pout, "%llu\n", paddr);
		}

		if (i % 100 == 0) {
			fflush(pout);
		}
	}

	// Close the files
	fclose(pin);
	fclose(pout);

	return 0;
}