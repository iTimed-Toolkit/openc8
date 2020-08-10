/*
 * CPU power state driver for Hx SoCs
 *
 * Copyright (C) 2020 Corellium LLC
 */

#include "bootrom_func.h"

#define REG_ACC_CLK_L2FEAT      0x0018
#define REG_ACC_CLK_PSTATE      0x0020
#define REG_ACC_CLK_ENABLE2     0x0048
#define REG_ACC_CLK_ENABLE1     0x0660
#define REG_ACC_CLK_ENABLE3     0x06b8
#define REG_DVFM_VSET_LO        0x1220
#define REG_DVFM_VSET_HI        0x1228
#define REG_DVFM_SET1_LO        0x1100
#define REG_DVFM_SET1_HI        0x1108
#define REG_DVFM_SET2_LO        0x1120
#define REG_DVFM_SET2_HI        0x1128
#define REG_DVFM_MASK_LO        0x1420
#define REG_DVFM_MASK_HI        0x1428
#define REG_ACC_DBG_CTRL        0x2000
#define REG_PSINFO_SET1_DFLT    0x3000
#define REG_PSINFO_SET2_DFLT    0x3008
#define REG_PSINFO_GET1(ps)    (0x3000+0x20*(ps))
#define REG_PSINFO_GET2(ps)    (0x3008+0x20*(ps))
#define REG_PSINFO_SET1         0x31E0
#define REG_PSINFO_SET2         0x31E8
#define REG_MCCPS_IDX           0x4000
#define REG_MCCPS_GET(ps)      (0x4018+0x10*(ps))
#define REG_PSTATE_DISABLE      0x5080
#define REG_PSTATE_CTRL         0x6000
#define REG_PSTATE_SET3         0x6008
#define REG_PSTATE_SET2         0x6010
#define REG_PSTATE_SET1         0x6018
#define REG_CPUPM_CPU0          0x7000
#define REG_CPUPM_CPU1          0x7200

GLOBAL_LONG_ARR(bases,
                0x202f20000, 0x202f48000, 0x202f50000, 0x202f80000,
                0x20e068000, 0x20e09c000, 0x20e0a0000, 0x20e0c8000)

#define NUM_STATES_FAST         12
GLOBAL_CHAR_ARR(hx_cpufreq_dvfm_vset_fast,
                0x00, 0x00, 0x37, 0x3B, 0x4F, 0x40,
                0x4C, 0x5F, 0x71, 0x8F, 0xB5, 0xB5)
GLOBAL_CHAR_ARR(hx_cpufreq_dvfm_set1_fast,
                0x00, 0x00, 0x0A, 0x05, 0x04, 0x1F,
                0x15, 0x10, 0x0D, 0x0B, 0x09, 0x09)
GLOBAL_CHAR_ARR(hx_cpufreq_dvfm_set2_fast,
                0x00, 0x00, 0x12, 0x0A, 0x06, 0x09,
                0x07, 0x05, 0x04, 0x03, 0x03, 0x03)

#define NUM_STATES_SLOW         9
GLOBAL_CHAR_ARR(hx_cpufreq_dvfm_vset_slow,
                0x00, 0x00, 0x37, 0x3B, 0x4F,
                0x40, 0x4C, 0x5F, 0x71)
GLOBAL_CHAR_ARR(hx_cpufreq_dvfm_set1_slow,
                0x00, 0x00, 0x0A, 0x05, 0x04,
                0x1F, 0x15, 0x10, 0x0D)
GLOBAL_CHAR_ARR(hx_cpufreq_dvfm_set2_slow,
                0x00, 0x00, 0x12, 0x0A, 0x06,
                0x09, 0x07, 0x05, 0x04)

struct cpufreq_frequency_table
{
    unsigned int flags;
    unsigned int driver_data; /* driver specific data, not used by core */
    unsigned int frequency; /* kHz - doesn't need to be in ascending order */
};

struct hx_cpufreq_config
{
    unsigned num_states;
    const uint8_t *dvfm_vset;
    const uint8_t *dvfm_set1;
    const uint8_t *dvfm_set2;
    struct cpufreq_frequency_table *cpufreq_table;
};

#define NUM_BASES       8
struct hx_cpufreq_data
{
    void *base[NUM_BASES];
    const struct hx_cpufreq_config *cfg;
};

#define CPUFREQ_ENTRY_INVALID   0
#define CPUFREQ_TABLE_END       -1


GLOBAL_INT_ARR(hx_cpufreq_table_fast_values,
               CPUFREQ_ENTRY_INVALID, CPUFREQ_ENTRY_INVALID,
               149000, 275000, 410000, 756000, 1056000,
               1356000, 1644000, 1944000, 2244000, 2340000,
               CPUFREQ_TABLE_END)

GLOBAL_PTR(hx_cpufreq_table_fast, 0)
GLOBAL_PTR(hx_cpufreq_config_fast, 0)

GLOBAL_INT_ARR(hx_cpufreq_table_slow_values,
               CPUFREQ_ENTRY_INVALID, CPUFREQ_ENTRY_INVALID,
               149000, 275000, 410000, 756000, 1056000,
               1356000, 1644000, CPUFREQ_TABLE_END)

GLOBAL_PTR(hx_cpufreq_table_slow, 0)
GLOBAL_PTR(hx_cpufreq_config_slow, 0)

GLOBAL_PTR(hx_cpufreq_current, 0);

static inline uint32_t readl(void *addr)
{
    return *((uint32_t *) addr);
}

static inline uint64_t readq(void *addr)
{
    return *((uint64_t *) addr);
}

static inline void writel(uint32_t val, void *addr)
{
    *((uint32_t *) addr) = val;
}

static inline void writeq(uint64_t val, void *addr)
{
    *((uint64_t *) addr) = val;
}

static inline void *hx_cpufreq_reg(unsigned reg)
{
    struct hx_cpufreq_data *curr = GET_GLOBAL(hx_cpufreq_current);
    return curr->base[reg >> 12u] + (reg & 0xFFFu);
}

PAYLOAD_SECTION
static uint32_t hx_cpufreq_readl(unsigned reg)
{
    return readl(hx_cpufreq_reg(reg));
}

PAYLOAD_SECTION
static uint64_t hx_cpufreq_readq(unsigned reg)
{
    return readq(hx_cpufreq_reg(reg));
}

PAYLOAD_SECTION
static void hx_cpufreq_writel(unsigned reg, uint32_t val)
{
    writel(val, hx_cpufreq_reg(reg));
}

PAYLOAD_SECTION
static void hx_cpufreq_writeq(unsigned reg, uint64_t val)
{
    writeq(val, hx_cpufreq_reg(reg));
}

PAYLOAD_SECTION
static void hx_cpufreq_rmwl(unsigned reg, uint32_t clr, uint32_t set)
{
    void *ptr = hx_cpufreq_reg(reg);
    writel((readl(ptr) & ~clr) | set, ptr);
}

PAYLOAD_SECTION
static void hx_cpufreq_rmwq(unsigned reg, uint64_t clr, uint64_t set)
{
    void *ptr = hx_cpufreq_reg(reg);
    writeq((readq(ptr) & ~clr) | set, ptr);
}

PAYLOAD_SECTION
__attribute__ ((noinline))
static int hx_cpufreq_set_target(unsigned int index)
{
    struct hx_cpufreq_data *curr = GET_GLOBAL(hx_cpufreq_current);
    if(index < 2)
        index = 2;
    if(index > curr->cfg->num_states - 1)
        index = curr->cfg->num_states - 1;

    hx_cpufreq_writeq(REG_PSINFO_SET1, (hx_cpufreq_readq(REG_PSINFO_GET1(index)) & 0xFF00000000800000ul) |
                                           (hx_cpufreq_readq(REG_PSINFO_SET1_DFLT) & ~0xFF00000000800000ul));
    hx_cpufreq_writeq(REG_PSINFO_SET2, (hx_cpufreq_readq(REG_PSINFO_GET2(index)) & 0x000000FF00000000ul) |
                                           (hx_cpufreq_readq(REG_PSINFO_SET2_DFLT) & ~0x000000FF00000000ul));
    hx_cpufreq_rmwq(REG_ACC_CLK_PSTATE, 0xF00Ful, index | (index << 12) | 0x20F0000ul);

    return 0;
}

PAYLOAD_SECTION
void hx_cpufreq_write_dvfm_set(unsigned rlo, unsigned rhi, const uint8_t *set, unsigned len)
{
    uint64_t val[2] = {0}, ent;
    unsigned i;
    for(i = 0; i < len; i++)
    {
        ent = set ? set[i] : 0xFF;
        val[i >> 3] |= ent << ((i & 7) << 3);
    }

    hx_cpufreq_writeq(rlo, val[0]);
    hx_cpufreq_writeq(rhi, val[1]);
}

PAYLOAD_SECTION
int hx_cpufreq_init()
{
    struct hx_cpufreq_data *hc = GET_GLOBAL(hx_cpufreq_current);

    hx_cpufreq_write_dvfm_set(REG_DVFM_VSET_LO, REG_DVFM_VSET_HI, hc->cfg->dvfm_vset, hc->cfg->num_states); // works
    hx_cpufreq_write_dvfm_set(REG_DVFM_SET1_LO, REG_DVFM_SET1_HI, hc->cfg->dvfm_set1, hc->cfg->num_states); // works
    hx_cpufreq_write_dvfm_set(REG_DVFM_SET2_LO, REG_DVFM_SET2_HI, hc->cfg->dvfm_set2, hc->cfg->num_states); // works
    hx_cpufreq_write_dvfm_set(REG_DVFM_MASK_LO, REG_DVFM_MASK_HI, 0, hc->cfg->num_states); // works
    hx_cpufreq_writeq(REG_ACC_CLK_ENABLE1, 0x15); // works
    hx_cpufreq_rmwl(REG_PSTATE_CTRL, 0x1F00, 0); // works
    hx_cpufreq_writel(REG_PSTATE_SET1, hx_cpufreq_readq(REG_PSINFO_GET1(2)) >> 56u); // works
    hx_cpufreq_writel(REG_PSTATE_SET2, (hx_cpufreq_readq(REG_PSINFO_GET2(2)) >> 32u) & 0xFFu); // works
    hx_cpufreq_writel(REG_PSTATE_SET3, (hx_cpufreq_readl(REG_MCCPS_GET(hx_cpufreq_readl(REG_MCCPS_IDX) & 15u)) >> 12u) & 0xFFu); // works
    hx_cpufreq_rmwq(REG_ACC_CLK_PSTATE, 0x400000, 0); // works
    hx_cpufreq_rmwq(REG_ACC_CLK_ENABLE2, 0, 1); // works
    hx_cpufreq_rmwl(REG_CPUPM_CPU0, 0, 1); // works
    hx_cpufreq_rmwl(REG_CPUPM_CPU1, 0, 1); // works
    hx_cpufreq_rmwq(REG_ACC_CLK_ENABLE3, 0, 1ul << 63u); // works
//    hx_cpufreq_writel(REG_ACC_CLK_L2FEAT, 0xFFFFFFFF); // FAILS addr 0x202f20018
    hx_cpufreq_rmwq(REG_ACC_DBG_CTRL, 0, 0x7C000);
    hx_cpufreq_rmwl(REG_PSTATE_DISABLE, 0x80000000u, 0);

    task_pause(10000);

    hx_cpufreq_rmwq(REG_PSINFO_SET1_DFLT, 0xFFul << 56u, hx_cpufreq_readq(REG_PSINFO_GET1(2)) & (0xFFul << 56u));
    hx_cpufreq_rmwq(REG_PSINFO_SET2_DFLT, 0xFFul << 32u, hx_cpufreq_readq(REG_PSINFO_GET2(2)) & (0xFFul << 32u));
    hx_cpufreq_rmwq(REG_ACC_CLK_PSTATE, 0, 0x60002000);

    task_pause(10000);

    return 0;
}

PAYLOAD_SECTION
int hx_cpufreq_setup()
{
    int i;
    struct cpufreq_frequency_table *table;
    struct hx_cpufreq_config *config;
    struct hx_cpufreq_data *data;

    /* Fast config */
    table = dev_malloc(sizeof(struct cpufreq_frequency_table) * (NUM_STATES_FAST + 1));
    for(i = 0; i < NUM_STATES_FAST + 1; i++)
        table[i].frequency = GET_GLOBAL_AT(hx_cpufreq_table_fast_values, i);

    SET_GLOBAL(hx_cpufreq_table_fast, table);

    config = dev_malloc(sizeof(struct hx_cpufreq_config));
    config->num_states = NUM_STATES_FAST;
    config->dvfm_vset = (uint8_t *) GET_PTR_TO_GLOBAL(hx_cpufreq_dvfm_vset_fast);
    config->dvfm_set1 = (uint8_t *) GET_PTR_TO_GLOBAL(hx_cpufreq_dvfm_set1_fast);
    config->dvfm_set2 = (uint8_t *) GET_PTR_TO_GLOBAL(hx_cpufreq_dvfm_set2_fast);

    SET_GLOBAL(hx_cpufreq_config_fast, config);

    /* Slow config */
    table = dev_malloc(sizeof(struct cpufreq_frequency_table) * (NUM_STATES_SLOW + 1));
    for(i = 0; i < NUM_STATES_SLOW + 1; i++)
        table[i].frequency = GET_GLOBAL_AT(hx_cpufreq_table_slow_values, i);

    SET_GLOBAL(hx_cpufreq_table_slow, table);

    config = dev_malloc(sizeof(struct hx_cpufreq_config));
    config->num_states = NUM_STATES_SLOW;
    config->dvfm_vset = (uint8_t *) GET_PTR_TO_GLOBAL(hx_cpufreq_dvfm_vset_slow);
    config->dvfm_set1 = (uint8_t *) GET_PTR_TO_GLOBAL(hx_cpufreq_dvfm_set1_slow);
    config->dvfm_set2 = (uint8_t *) GET_PTR_TO_GLOBAL(hx_cpufreq_dvfm_set2_slow);

    SET_GLOBAL(hx_cpufreq_config_slow, config);

    data = dev_malloc(sizeof(struct hx_cpufreq_data));
    for(i = 0; i < NUM_BASES; i++)
        data->base[i] = (void *) GET_GLOBAL_AT(bases, i);

    data->cfg = GET_GLOBAL(hx_cpufreq_config_fast);
    SET_GLOBAL(hx_cpufreq_current, data);

    return 0;
}

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE)

extern int busy_loop(uint64_t amount);

GLOBAL_CHAR(initialized, 0);
void _start()
{
    int i;
    uint64_t time_fast, time_slow, start;
    uint64_t sum = 0, amount = *((uint64_t *) GET_GLOBAL(addr_dfu_img_base));

    if(GET_GLOBAL(initialized) == 0)
    {
        hx_cpufreq_setup();
        hx_cpufreq_init();
        SET_GLOBAL(initialized, 1);
    }

    hx_cpufreq_set_target(12);
    start = get_ticks();
    sum += busy_loop(amount);
    time_fast = get_ticks() - start;

    hx_cpufreq_set_target(2);
    start = get_ticks();
    sum += busy_loop(amount);
    time_slow = get_ticks() - start;

    ((uint64_t *) GET_GLOBAL(addr_dfu_img_base))[0] = time_fast;
    ((uint64_t *) GET_GLOBAL(addr_dfu_img_base))[1] = time_slow;
    ((uint64_t *) GET_GLOBAL(addr_dfu_img_base))[2] = sum;
}