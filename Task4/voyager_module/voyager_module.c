#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/time.h>

MODULE_DESCRIPTION("Kernel Module");
MODULE_AUTHOR("NikitaEffel");

#define PROC_FILENAME "voyager_days"
#define VOYAGER_LAUNCH_YEAR 1977
#define VOYAGER_LAUNCH_MONTH 8
#define VOYAGER_LAUNCH_DAY 20
#define VOYAGER_LAUNCH_HOUR 21
#define VOYAGER_LAUNCH_MINUTE 39

static struct proc_dir_entry *proc_entry = NULL;
static struct timespec64 voyager_start_time;

static long calculate_days_since_voyager_launch(void) {
    struct timespec64 current_time;
    long long elapsed_seconds;
    long days_elapsed;

    ktime_get_real_ts64(&current_time);

    elapsed_seconds = current_time.tv_sec - voyager_start_time.tv_sec;
    days_elapsed = elapsed_seconds / (60 * 60 * 24);

    return days_elapsed;
}

static ssize_t proc_file_read(struct file *file, char __user *user_buffer, size_t buffer_size, loff_t *position) {
    char output_buffer[32];
    size_t output_size;
    long days_elapsed;

    if (*position > 0)
        return 0;

    days_elapsed = calculate_days_since_voyager_launch();
    output_size = snprintf(output_buffer, sizeof(output_buffer), "%ld\n", days_elapsed);

    if (copy_to_user(user_buffer, output_buffer, output_size))
        return -EFAULT;

    *position += output_size;
    return output_size;
}

static const struct proc_ops proc_file_operations = {
    .proc_read = proc_file_read,
};

static int __init voyager_module_init(void) {
    struct tm launch_time_tm = {
        .tm_year = VOYAGER_LAUNCH_YEAR - 1900,
        .tm_mon = VOYAGER_LAUNCH_MONTH - 1,
        .tm_mday = VOYAGER_LAUNCH_DAY,
        .tm_hour = VOYAGER_LAUNCH_HOUR,
        .tm_min = VOYAGER_LAUNCH_MINUTE,
        .tm_sec = 0
    };
    time64_t launch_time_seconds;

    launch_time_seconds = mktime64(launch_time_tm.tm_year + 1900, launch_time_tm.tm_mon + 1, launch_time_tm.tm_mday,
                                   launch_time_tm.tm_hour, launch_time_tm.tm_min, launch_time_tm.tm_sec);

    voyager_start_time.tv_sec = launch_time_seconds;
    voyager_start_time.tv_nsec = 0;

    proc_entry = proc_create(PROC_FILENAME, 0644, NULL, &proc_file_operations);
    if (!proc_entry) {
        pr_err("Failed to create /proc/%s\n", PROC_FILENAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s successfully created\n", PROC_FILENAME);
    return 0;
}

static void __exit voyager_module_exit(void) {
    proc_remove(proc_entry);
    pr_info("/proc/%s successfully removed\n", PROC_FILENAME);
}

module_init(voyager_module_init);
module_exit(voyager_module_exit);

MODULE_LICENSE("GPL");
