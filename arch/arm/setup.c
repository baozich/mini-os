#include <mini-os/os.h>
#include <mini-os/kernel.h>
#include <mini-os/gic.h>
#include <mini-os/console.h>
#include <xen/xen.h>
#include <xen/memory.h>
#include <xen/hvm/params.h>
#include <arch_mm.h>
#include <libfdt.h>

/*
 * This structure contains start-of-day info, such as pagetable base pointer,
 * address of the shared_info structure, and things like that.
 * On x86, the hypervisor passes it to us. On ARM, we fill it in ourselves.
 */
union start_info_union start_info_union;

/*
 * Shared page for communicating with the hypervisor.
 * Events flags go here, for example.
 */
shared_info_t *HYPERVISOR_shared_info;

extern char shared_info_page[PAGE_SIZE];
extern lpae_t boot_l1_pgtable[512];
extern lpae_t fixmap_pgtable[512];

void *device_tree;

static inline void set_pgt_entry(lpae_t *ptr, lpae_t val)
{
    *ptr = val;
    dsb(ishst);
    isb();
}

static int hvm_get_parameter(int idx, uint64_t *value)
{
    struct xen_hvm_param xhv;
    int ret;

    xhv.domid = DOMID_SELF;
    xhv.index = idx;
    ret = HYPERVISOR_hvm_op(HVMOP_get_param, &xhv);
    if (ret < 0) {
        BUG();
    }
    *value = xhv.value;
    return ret;
}

static xen_pfn_t map_console(xen_pfn_t mfn)
{
    paddr_t phys;

    phys = PFN_PHYS(mfn);
    xprintk("map_console, phys = 0x%lx\n", phys);

    set_pgt_entry(&fixmap_pgtable[l2_pgt_idx(FIX_CON_START)],
                  ((phys & L2_MASK) | BLOCK_DEV_ATTR | L2_BLOCK));

    return (xen_pfn_t) (FIX_CON_START + (phys & L2_OFFSET));
}

static void get_console(void)
{
    uint64_t v = -1;

    hvm_get_parameter(HVM_PARAM_CONSOLE_EVTCHN, &v);
    start_info.console.domU.evtchn = v;

    hvm_get_parameter(HVM_PARAM_CONSOLE_PFN, &v);
#if defined(__aarch64__)
    start_info.console.domU.mfn = map_console(v);
#else
    start_info.console.domU.mfn = v;
#endif

    printk("Console is on port %d\n", start_info.console.domU.evtchn);
    printk("Console ring is at mfn %lx\n", (unsigned long)start_info.console.domU.mfn);
}

void get_xenbus(void)
{
    uint64_t value;

    if (hvm_get_parameter(HVM_PARAM_STORE_EVTCHN, &value))
        BUG();

    start_info.store_evtchn = (int)value;

    if(hvm_get_parameter(HVM_PARAM_STORE_PFN, &value))
        BUG();
    start_info.store_mfn = (unsigned long)value;
}

/*
 * Map device_tree (paddr) to FIX_FDT_START (vaddr)
 */
static void *map_fdt(paddr_t device_tree)
{
    /*
     * FIXME: To deal with the 2M alignment, only 4KB space is usable
     * because device_tree is aligned to a (2M - 4KB) address.
     */
    set_pgt_entry(&boot_l1_pgtable[l1_pgt_idx(FIX_FDT_START)],
                  (to_phys(fixmap_pgtable) | L1_TABLE));
    set_pgt_entry(&fixmap_pgtable[l2_pgt_idx(FIX_FDT_START)],
                  ((device_tree & L2_MASK) | BLOCK_DEF_ATTR | L2_BLOCK));

    return (void *)(FIX_FDT_START + (device_tree & L2_OFFSET));
}


/*
 * INITIAL C ENTRY POINT.
 */
void arch_init(void *dtb_pointer, paddr_t physical_offset)
{
    struct xen_add_to_physmap xatp;
    int r;

    memset(&__bss_start, 0, &_end - &__bss_start);

    physical_address_offset = physical_offset;

    dtb_pointer = map_fdt((paddr_t) dtb_pointer);

    printk("Checking DTB at %p...\n", dtb_pointer);

    if ((r = fdt_check_header(dtb_pointer))) {
        printk("Invalid DTB from Xen: %s\n", fdt_strerror(r));
        BUG();
    }
    device_tree = dtb_pointer;

    /* Map shared_info page */
    xatp.domid = DOMID_SELF;
    xatp.idx = 0;
    xatp.space = XENMAPSPACE_shared_info;
    xatp.gpfn = virt_to_pfn(shared_info_page);
    if (HYPERVISOR_memory_op(XENMEM_add_to_physmap, &xatp) != 0)
        BUG();
    HYPERVISOR_shared_info = (struct shared_info *)shared_info_page;

    /* Fill in start_info */
    get_console();
    get_xenbus();

    start_kernel();
}

void
arch_fini(void)
{
}

void
arch_do_exit(void)
{
}
