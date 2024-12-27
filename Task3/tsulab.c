#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomsk State University");
MODULE_DESCRIPTION("TSU Linux Kernel Module with /proc File");

#define PROC_FILE_NAME "tsulab"
static struct proc_dir_entry *proc_file;

// Буфер содержимого файла
static char proc_content[] = "Tomsk State University Lab\n";

// Функция чтения файла
static ssize_t proc_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    size_t len = strlen(proc_content);
    if (*ppos > 0 || count < len)
        return 0; // Конец чтения

    if (copy_to_user(user_buf, proc_content, len))
        return -EFAULT;

    *ppos = len;
    return len;
}

// Объект файла
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read,
};
#else
static const struct file_operations proc_file_ops = {
    .read = proc_read,
};
#endif

// Инициализация модуля
static int __init tsu_module_init(void) {
    pr_info("Welcome to the Tomsk State University\n");

    // Создаём файл в /proc
    proc_file = proc_create(PROC_FILE_NAME, 0444, NULL, &proc_file_ops);
    if (!proc_file) {
        pr_err("Failed to create /proc/%s\n", PROC_FILE_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROC_FILE_NAME);
    return 0;
}

// Завершение работы модуля
static void __exit tsu_module_exit(void) {
    // Удаляем файл из /proc
    proc_remove(proc_file);
    pr_info("/proc/%s removed\n", PROC_FILE_NAME);
    pr_info("Tomsk State University forever!\n");
}

module_init(tsu_module_init);
module_exit(tsu_module_exit);
