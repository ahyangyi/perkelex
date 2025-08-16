// SPDX-License-Identifier: GPL-2.0

#define pr_fmt(fmt) "x86/split lock detection: " fmt

#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/cpuhotplug.h>
#include <asm/cpu_device_id.h>
#include <asm/cmdline.h>
#include <asm/traps.h>
#include <asm/cpu.h>
#include <asm/msr.h>

enum split_lock_detect_state {
	sld_off = 0,
	sld_warn,
	sld_fatal,
	sld_ratelimit,
};

/*
 * Default to sld_off because most systems do not support split lock detection.
 * sld_state_setup() will switch this to sld_warn on systems that support
 * split lock/bus lock detect, unless there is a command line override.
 */
static enum split_lock_detect_state sld_state __ro_after_init = sld_off;
static u64 msr_test_ctrl_cache __ro_after_init;

/*
 * With a name like MSR_TEST_CTL it should go without saying, but don't touch
 * MSR_TEST_CTL unless the CPU is one of the whitelisted models.  Writing it
 * on CPUs that do not support SLD can cause fireworks, even when writing '0'.
 */
static bool cpu_model_supports_sld __ro_after_init;

static const struct {
	const char			*option;
	enum split_lock_detect_state	state;
} sld_options[] __initconst = {
	{ "off",	sld_off   },
	{ "warn",	sld_warn  },
	{ "fatal",	sld_fatal },
	{ "ratelimit:", sld_ratelimit },
};

static struct ratelimit_state bld_ratelimit;

static unsigned int sysctl_sld_mitigate = 1;
static DEFINE_SEMAPHORE(buslock_sem, 1);

#ifdef CONFIG_PROC_SYSCTL
static const struct ctl_table sld_sysctls[] = {
	{
		.procname       = "split_lock_mitigate",
		.data           = &sysctl_sld_mitigate,
		.maxlen         = sizeof(unsigned int),
		.mode           = 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra1         = SYSCTL_ZERO,
		.extra2         = SYSCTL_ONE,
	},
};

static int __init sld_mitigate_sysctl_init(void)
{
	register_sysctl_init("kernel", sld_sysctls);
	return 0;
}

late_initcall(sld_mitigate_sysctl_init);
#endif

static inline bool match_option(const char *arg, int arglen, const char *opt)
{
	int len = strlen(opt), ratelimit;

	if (strncmp(arg, opt, len))
		return false;

	/*
	 * Min ratelimit is 1 bus lock/sec.
	 * Max ratelimit is 1000 bus locks/sec.
	 */
	if (sscanf(arg, "ratelimit:%d", &ratelimit) == 1 &&
	    ratelimit > 0 && ratelimit <= 1000) {
		ratelimit_state_init(&bld_ratelimit, HZ, ratelimit);
		ratelimit_set_flags(&bld_ratelimit, RATELIMIT_MSG_ON_RELEASE);
		return true;
	}

	return len == arglen;
}

static bool split_lock_verify_msr(bool on)
{
	u64 ctrl, tmp;

	if (rdmsrq_safe(MSR_TEST_CTRL, &ctrl))
		return false;
	if (on)
		ctrl |= MSR_TEST_CTRL_SPLIT_LOCK_DETECT;
	else
		ctrl &= ~MSR_TEST_CTRL_SPLIT_LOCK_DETECT;
	if (wrmsrq_safe(MSR_TEST_CTRL, ctrl))
		return false;
	rdmsrq(MSR_TEST_CTRL, tmp);
	return ctrl == tmp;
}

static void __init sld_state_setup(void)
{
}

static void __init __split_lock_setup(void)
{
	if (!split_lock_verify_msr(false)) {
		pr_info("MSR access failed: Disabled\n");
		return;
	}

	rdmsrq(MSR_TEST_CTRL, msr_test_ctrl_cache);

	if (!split_lock_verify_msr(true)) {
		pr_info("MSR access failed: Disabled\n");
		return;
	}

	/* Restore the MSR to its cached value. */
	wrmsrq(MSR_TEST_CTRL, msr_test_ctrl_cache);

	setup_force_cpu_cap(X86_FEATURE_SPLIT_LOCK_DETECT);
}

/*
 * MSR_TEST_CTRL is per core, but we treat it like a per CPU MSR. Locking
 * is not implemented as one thread could undo the setting of the other
 * thread immediately after dropping the lock anyway.
 */
static void sld_update_msr(bool on)
{
	u64 test_ctrl_val = msr_test_ctrl_cache;

	if (on)
		test_ctrl_val |= MSR_TEST_CTRL_SPLIT_LOCK_DETECT;

	wrmsrq(MSR_TEST_CTRL, test_ctrl_val);
}

void split_lock_init(void)
{
}

static void __split_lock_reenable_unlock(struct work_struct *work)
{
	sld_update_msr(true);
	up(&buslock_sem);
}

static DECLARE_DELAYED_WORK(sl_reenable_unlock, __split_lock_reenable_unlock);

static void __split_lock_reenable(struct work_struct *work)
{
	sld_update_msr(true);
}
/*
 * In order for each CPU to schedule its delayed work independently of the
 * others, delayed work struct must be per-CPU. This is not required when
 * sysctl_sld_mitigate is enabled because of the semaphore that limits
 * the number of simultaneously scheduled delayed works to 1.
 */
static DEFINE_PER_CPU(struct delayed_work, sl_reenable);

/*
 * Per-CPU delayed_work can't be statically initialized properly because
 * the struct address is unknown. Thus per-CPU delayed_work structures
 * have to be initialized during kernel initialization and after calling
 * setup_per_cpu_areas().
 */
static int __init setup_split_lock_delayed_work(void)
{
	unsigned int cpu;

	for_each_possible_cpu(cpu) {
		struct delayed_work *work = per_cpu_ptr(&sl_reenable, cpu);

		INIT_DELAYED_WORK(work, __split_lock_reenable);
	}

	return 0;
}
pure_initcall(setup_split_lock_delayed_work);

/*
 * If a CPU goes offline with pending delayed work to re-enable split lock
 * detection then the delayed work will be executed on some other CPU. That
 * handles releasing the buslock_sem, but because it executes on a
 * different CPU probably won't re-enable split lock detection. This is a
 * problem on HT systems since the sibling CPU on the same core may then be
 * left running with split lock detection disabled.
 *
 * Unconditionally re-enable detection here.
 */
static int splitlock_cpu_offline(unsigned int cpu)
{
	sld_update_msr(true);

	return 0;
}

static void split_lock_warn(unsigned long ip)
{
	struct delayed_work *work;
	int cpu;

	cpu = get_cpu();
	work = per_cpu_ptr(&sl_reenable, cpu);
	schedule_delayed_work_on(cpu, work, 2);

	/* Disable split lock detection on this CPU to make progress */
	sld_update_msr(false);
	put_cpu();
}

bool handle_guest_split_lock(unsigned long ip)
{
    return true;
}
EXPORT_SYMBOL_GPL(handle_guest_split_lock);

void bus_lock_init(void)
{
	u64 val;

	if (!boot_cpu_has(X86_FEATURE_BUS_LOCK_DETECT))
		return;

	rdmsrq(MSR_IA32_DEBUGCTLMSR, val);

	val |= DEBUGCTLMSR_BUS_LOCK_DETECT;

	wrmsrq(MSR_IA32_DEBUGCTLMSR, val);
}

bool handle_user_split_lock(struct pt_regs *regs, long error_code)
{
	return true;
}

void handle_bus_lock(struct pt_regs *regs)
{
}

/*
 * CPU models that are known to have the per-core split-lock detection
 * feature even though they do not enumerate IA32_CORE_CAPABILITIES.
 */
static const struct x86_cpu_id split_lock_cpu_ids[] __initconst = {
	X86_MATCH_VFM(INTEL_ICELAKE_X,	0),
	X86_MATCH_VFM(INTEL_ICELAKE_L,	0),
	X86_MATCH_VFM(INTEL_ICELAKE_D,	0),
	{}
};

static void __init split_lock_setup(struct cpuinfo_x86 *c)
{
	const struct x86_cpu_id *m;
	u64 ia32_core_caps;

	if (boot_cpu_has(X86_FEATURE_HYPERVISOR))
		return;

	/* Check for CPUs that have support but do not enumerate it: */
	m = x86_match_cpu(split_lock_cpu_ids);
	if (m)
		goto supported;

	if (!cpu_has(c, X86_FEATURE_CORE_CAPABILITIES))
		return;

	/*
	 * Not all bits in MSR_IA32_CORE_CAPS are architectural, but
	 * MSR_IA32_CORE_CAPS_SPLIT_LOCK_DETECT is.  All CPUs that set
	 * it have split lock detection.
	 */
	rdmsrq(MSR_IA32_CORE_CAPS, ia32_core_caps);
	if (ia32_core_caps & MSR_IA32_CORE_CAPS_SPLIT_LOCK_DETECT)
		goto supported;

	/* CPU is not in the model list and does not have the MSR bit: */
	return;

supported:
	cpu_model_supports_sld = true;
	__split_lock_setup();
}

static void sld_state_show(void)
{
}

void __init sld_setup(struct cpuinfo_x86 *c)
{
	split_lock_setup(c);
	sld_state_setup();
	sld_state_show();
}
