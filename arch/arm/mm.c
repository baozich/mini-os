#include <mini-os/console.h>
#include <xen/memory.h>
#include <arch_mm.h>
#include <mini-os/hypervisor.h>
#include <libfdt.h>
#include <lib.h>

#if defined(__aarch64__)
extern char stack[];
extern lpae_t boot_l1_pgtable[512];
extern lpae_t boot_l2_pgtable[512];
extern lpae_t fixmap_pgtable[512];
extern lpae_t idmap_pgtable[512];
#endif

paddr_t physical_address_offset;

unsigned long allocate_ondemand(unsigned long n, unsigned long alignment)
{
    // FIXME
    BUG();
}

static inline void set_pgt_entry(lpae_t *ptr, lpae_t val)
{
    *ptr = val;
    dsb(ishst);
    isb();
}

static void alloc_init_pud(lpae_t *pgd, unsigned long vaddr,
                           unsigned long vend, paddr_t phys)
{
    lpae_t *pud;
    int count = 0;

    /*
     * FIXME: to support >1GiB physical memory, need to allocate a new
     * level-2 page table from boot memory.
     */
    if (!(*pgd)) {
       BUG();
    }

    pud = (lpae_t *)to_virt((*pgd) & ~ATTR_MASK_L) + l2_pgt_idx(vaddr);
    do {
        set_pgt_entry(pud, (phys & L2_MASK) | BLOCK_DEF_ATTR | L2_BLOCK);
        vaddr += L2_SIZE;
        phys += L2_SIZE;
        pud++;
        count++;
    } while (vaddr < vend);

    return;
}

static void build_pagetable(unsigned long start_pfn, unsigned long max_pfn)
{
    paddr_t start_paddr, end_paddr;
    unsigned long start_vaddr, end_vaddr;
    unsigned long vaddr, next;
    lpae_t *pgd;

    start_paddr = PFN_PHYS(start_pfn);
    end_paddr = PFN_PHYS(start_pfn + max_pfn);

    start_vaddr = (unsigned long)to_virt(start_paddr);
    end_vaddr = (unsigned long)to_virt(end_paddr);
    pgd = &boot_l1_pgtable[l1_pgt_idx(start_vaddr)];

    vaddr = start_vaddr;
    do {
        next = (vaddr + L1_SIZE);
        if (next > end_vaddr)
            next = end_vaddr;
        alloc_init_pud(pgd, vaddr, next, start_paddr);
        start_paddr += next - vaddr;
        vaddr = next;
        pgd++;
    } while (vaddr != end_vaddr);

    return;
}

void arch_init_mm(unsigned long *start_pfn_p, unsigned long *max_pfn_p)
{
    int memory;
    int prop_len = 0;
    unsigned long end;
    paddr_t mem_base;
    uint64_t mem_size;
    uint64_t heap_len;
    const uint64_t *regs;

    printk("    _text: %p(VA)\n", &_text);
    printk("    _etext: %p(VA)\n", &_etext);
    printk("    _erodata: %p(VA)\n", &_erodata);
    printk("    _edata: %p(VA)\n", &_edata);
    printk("    stack start: %p(VA)\n", stack);
    printk("    _end: %p(VA)\n", &_end);

    if (fdt_num_mem_rsv(device_tree) != 0)
        printk("WARNING: reserved memory not supported!\n");

    memory = fdt_node_offset_by_prop_value(device_tree, -1, "device_type", "memory", sizeof("memory"));
    if (memory < 0) {
        printk("No memory found in FDT!\n");
        BUG();
    }

    /* Xen will always provide us at least one bank of memory.
     * Mini-OS will use the first bank for the time-being. */
    regs = fdt_getprop(device_tree, memory, "reg", &prop_len);

    /* The property must contain at least the start address
     * and size, each of which is 8-bytes. */
    if (regs == NULL || prop_len < 16) {
        printk("Bad 'reg' property: %p %d\n", regs, prop_len);
        BUG();
    }

    end = (unsigned long) &_end;
    mem_base = fdt64_to_cpu(regs[0]);
    mem_size = fdt64_to_cpu(regs[1]);
    printk("Found memory at 0x%llx (len 0x%llx)\n",
            (unsigned long long) mem_base, (unsigned long long) mem_size);

    build_pagetable(PHYS_PFN(mem_base), PHYS_PFN(mem_size));

    BUG_ON(to_virt(mem_base) > (void *) &_text);          /* Our image isn't in our RAM! */

    *start_pfn_p = PFN_UP(to_phys(end));
    heap_len = mem_size - (PFN_PHYS(*start_pfn_p) - mem_base);
    *max_pfn_p = *start_pfn_p + PFN_DOWN(heap_len);
    printk("Using pages %lu to %lu as free space for heap.\n", *start_pfn_p, *max_pfn_p);

#if defined(__arm__)
    /* The device tree is probably in memory that we're about to hand over to the page
     * allocator, so move it to the end and reserve that space.
     */
    uint32_t fdt_size = fdt_totalsize(device_tree);
    void *new_device_tree = to_virt(((*max_pfn_p << PAGE_SHIFT) - fdt_size) & PAGE_MASK);
    if (new_device_tree != device_tree) {
        memmove(new_device_tree, device_tree, fdt_size);
    }
    device_tree = new_device_tree;
    *max_pfn_p = to_phys(new_device_tree) >> PAGE_SHIFT;
#endif
}

void arch_init_p2m(unsigned long max_pfn)
{
}

void arch_init_demand_mapping_area(unsigned long cur_pfn)
{
}

/* Get Xen's suggested physical page assignments for the grant table. */
static paddr_t get_gnttab_base(void)
{
    int hypervisor;
    int len = 0;
    const uint64_t *regs;
    paddr_t gnttab_base;

    hypervisor = fdt_node_offset_by_compatible(device_tree, -1, "xen,xen");
    BUG_ON(hypervisor < 0);

    regs = fdt_getprop(device_tree, hypervisor, "reg", &len);
    /* The property contains the address and size, 8-bytes each. */
    if (regs == NULL || len < 16) {
        printk("Bad 'reg' property: %p %d\n", regs, len);
        BUG();
    }

    gnttab_base = fdt64_to_cpu(regs[0]);

    printk("FDT suggests grant table base %llx\n", (unsigned long long) gnttab_base);

    return gnttab_base;
}

grant_entry_t *arch_init_gnttab(int nr_grant_frames)
{
    struct xen_add_to_physmap xatp;
    struct gnttab_setup_table setup;
    xen_pfn_t frames[nr_grant_frames];
    paddr_t gnttab_table;
    int i, rc;

    gnttab_table = get_gnttab_base();

    for (i = 0; i < nr_grant_frames; i++)
    {
        xatp.domid = DOMID_SELF;
        xatp.size = 0;      /* Seems to be unused */
        xatp.space = XENMAPSPACE_grant_table;
        xatp.idx = i;
        xatp.gpfn = (gnttab_table >> PAGE_SHIFT) + i;
        rc = HYPERVISOR_memory_op(XENMEM_add_to_physmap, &xatp);
        BUG_ON(rc != 0);
    }

    setup.dom = DOMID_SELF;
    setup.nr_frames = nr_grant_frames;
    set_xen_guest_handle(setup.frame_list, frames);
    HYPERVISOR_grant_table_op(GNTTABOP_setup_table, &setup, 1);
    if (setup.status != 0)
    {
        printk("GNTTABOP_setup_table failed; status = %d\n", setup.status);
        BUG();
    }

    return to_virt(gnttab_table);
}
