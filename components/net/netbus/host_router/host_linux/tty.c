#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include "tty.h"
#include "main.h"

#define TINY_TTY_MAJOR      0
#define TINY_TTY_MINORS     TTY_DEV_COUNT

struct tiny_serial {
    struct tty_struct   *tty;
    int         open_count;
    struct mutex    mutex;

    /* for tiocmget and tiocmset functions */
    int         msr;        /* MSR shadow */
    int         mcr;        /* MCR shadow */

    /* for ioctl fun */
    struct serial_struct    serial;
    wait_queue_head_t   wait;
    struct async_icount icount;
};

static struct tiny_serial *tiny_table[TINY_TTY_MINORS]; /* initially all NULL */
static struct tty_port tiny_tty_port[TINY_TTY_MINORS];

static int tiny_open(struct tty_struct *tty, struct file *file)
{
    struct tiny_serial *tiny;
    int index;

    /* initialize the pointer in case something fails */
    tty->driver_data = NULL;

    /* get the serial object associated with this tty pointer */
    index = tty->index;
    tiny = tiny_table[index];
    if (tiny == NULL) {
        /* first time accessing this device, let's create it */
        tiny = kmalloc(sizeof(*tiny), GFP_KERNEL);
        if (!tiny)
            return -ENOMEM;

        mutex_init(&tiny->mutex);
        tiny->open_count = 0;

        tiny_table[index] = tiny;
    }

    mutex_lock(&tiny->mutex);

    /* save our structure within the tty structure */
    tty->driver_data = tiny;
    tiny->tty = tty;

    ++tiny->open_count;

    mutex_unlock(&tiny->mutex);
    return 0;
}

static void do_close(struct tiny_serial *tiny)
{
    mutex_lock(&tiny->mutex);

    if (!tiny->open_count) {
        /* port was never opened */
        goto exit;
    }

    --tiny->open_count;
    if (tiny->open_count <= 0) {
        /* The port is being closed by the last user. */
        /* Do any hardware specific stuff here */
    }
exit:
    mutex_unlock(&tiny->mutex);
}

static void tiny_close(struct tty_struct *tty, struct file *file)
{
    struct tiny_serial *tiny = tty->driver_data;

    if (tiny)
        do_close(tiny);
}

static int tiny_write(struct tty_struct *tty,
              const unsigned char *buffer, int count)
{
    struct bl_eth_device *dev;
    struct tiny_serial *tiny = tty->driver_data;
    int retval = -EINVAL;
    struct sk_buff *skb;
    size_t buff_len;
    u16 idx;
    size_t l = 0;

    if (!tiny)
        return -ENODEV;

    mutex_lock(&tiny->mutex);

    if (!tiny->open_count)
        /* port was not opened */
        goto exit;

    if (!(count > 0)) {
        goto exit;
    }

    mutex_lock(&gl_dev.mutex);
    dev = gl_dev.eth_dev;
    if (dev == NULL) {
        mutex_unlock(&gl_dev.mutex);
        retval = -ENODEV;
        goto exit;
    }

    idx = tty->index;
    while (count > 0) {
        struct bl_tty_queued_data *qd;
        buff_len = min(2000, count);
        skb = alloc_skb(buff_len, GFP_KERNEL);
        if (!skb) {
            mutex_unlock(&gl_dev.mutex);
            retval = -ENOMEM;
            goto exit;
        }
        skb_put(skb, buff_len);
        qd = (struct bl_tty_queued_data *)skb->cb;
        qd->tty_id = idx;
        memcpy(skb->data, buffer + l, buff_len);
        skb_queue_tail(&dev->tty_msg_list, skb);

        l += buff_len;
        count -= buff_len;
    }
    queue_work(dev->txworkqueue, &dev->tx_work);

    mutex_unlock(&gl_dev.mutex);

    retval = l;

exit:
    mutex_unlock(&tiny->mutex);
    return retval;
}

static
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
unsigned
#endif
int tiny_write_room(struct tty_struct *tty)
{
    struct tiny_serial *tiny = tty->driver_data;
    int room = -EINVAL;

    if (!tiny)
        return -ENODEV;

    mutex_lock(&tiny->mutex);

    if (!tiny->open_count) {
        /* port was not opened */
        goto exit;
    }

    /* calculate how much room is left in the device */
    room = 255;

exit:
    mutex_unlock(&tiny->mutex);
    return room;
}

#define RELEVANT_IFLAG(iflag) ((iflag) & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
#define SET_TERMIOS_ARG1_CONST const
#else
#define SET_TERMIOS_ARG1_CONST
#endif

static void tiny_set_termios(struct tty_struct *tty, SET_TERMIOS_ARG1_CONST struct ktermios *old_termios)
{
    unsigned int cflag;

    cflag = tty->termios.c_cflag;

    /* check that they really want us to change something */
    if (old_termios) {
        if ((cflag == old_termios->c_cflag) &&
            (RELEVANT_IFLAG(tty->termios.c_iflag) ==
             RELEVANT_IFLAG(old_termios->c_iflag))) {
            pr_debug(" - nothing to change...\n");
            return;
        }
    }

    /* get the byte size */
    switch (cflag & CSIZE) {
    case CS5:
        pr_debug(" - data bits = 5\n");
        break;
    case CS6:
        pr_debug(" - data bits = 6\n");
        break;
    case CS7:
        pr_debug(" - data bits = 7\n");
        break;
    default:
    case CS8:
        pr_debug(" - data bits = 8\n");
        break;
    }

    /* determine the parity */
    if (cflag & PARENB)
        if (cflag & PARODD)
            pr_debug(" - parity = odd\n");
        else
            pr_debug(" - parity = even\n");
    else
        pr_debug(" - parity = none\n");

    /* figure out the stop bits requested */
    if (cflag & CSTOPB)
        pr_debug(" - stop bits = 2\n");
    else
        pr_debug(" - stop bits = 1\n");

    /* figure out the hardware flow control settings */
    if (cflag & CRTSCTS)
        pr_debug(" - RTS/CTS is enabled\n");
    else
        pr_debug(" - RTS/CTS is disabled\n");

    /* determine software flow control */
    /* if we are implementing XON/XOFF, set the start and
     * stop character in the device */
    if (I_IXOFF(tty) || I_IXON(tty)) {
        unsigned char stop_char  = STOP_CHAR(tty);
        unsigned char start_char = START_CHAR(tty);

        /* if we are implementing INBOUND XON/XOFF */
        if (I_IXOFF(tty))
            pr_debug(" - INBOUND XON/XOFF is enabled, "
                "XON = %2x, XOFF = %2x", start_char, stop_char);
        else
            pr_debug(" - INBOUND XON/XOFF is disabled");

        /* if we are implementing OUTBOUND XON/XOFF */
        if (I_IXON(tty))
            pr_debug(" - OUTBOUND XON/XOFF is enabled, "
                "XON = %2x, XOFF = %2x", start_char, stop_char);
        else
            pr_debug(" - OUTBOUND XON/XOFF is disabled");
    }

    /* get the baud rate wanted */
    pr_debug(" - baud rate = %d", tty_get_baud_rate(tty));
}

/* Our fake UART values */
#define MCR_DTR     0x01
#define MCR_RTS     0x02
#define MCR_LOOP    0x04
#define MSR_CTS     0x08
#define MSR_CD      0x10
#define MSR_RI      0x20
#define MSR_DSR     0x40

static int tiny_tiocmget(struct tty_struct *tty)
{
    struct tiny_serial *tiny = tty->driver_data;

    unsigned int result = 0;
    unsigned int msr = tiny->msr;
    unsigned int mcr = tiny->mcr;

    result = ((mcr & MCR_DTR)  ? TIOCM_DTR  : 0) |  /* DTR is set */
        ((mcr & MCR_RTS)  ? TIOCM_RTS  : 0) |   /* RTS is set */
        ((mcr & MCR_LOOP) ? TIOCM_LOOP : 0) |   /* LOOP is set */
        ((msr & MSR_CTS)  ? TIOCM_CTS  : 0) |   /* CTS is set */
        ((msr & MSR_CD)   ? TIOCM_CAR  : 0) |   /* Carrier detect is set*/
        ((msr & MSR_RI)   ? TIOCM_RI   : 0) |   /* Ring Indicator is set */
        ((msr & MSR_DSR)  ? TIOCM_DSR  : 0);    /* DSR is set */

    return result;
}

static int tiny_tiocmset(struct tty_struct *tty, unsigned int set,
             unsigned int clear)
{
    struct tiny_serial *tiny = tty->driver_data;
    unsigned int mcr = tiny->mcr;

    if (set & TIOCM_RTS)
        mcr |= MCR_RTS;
    if (set & TIOCM_DTR)
        mcr |= MCR_RTS;

    if (clear & TIOCM_RTS)
        mcr &= ~MCR_RTS;
    if (clear & TIOCM_DTR)
        mcr &= ~MCR_RTS;

    /* set the new MCR value in the device */
    tiny->mcr = mcr;
    return 0;
}

static int tiny_ioctl_tiocgserial(struct tty_struct *tty, unsigned int cmd,
              unsigned long arg)
{
    struct tiny_serial *tiny = tty->driver_data;

    if (cmd == TIOCGSERIAL) {
        struct serial_struct tmp;

        if (!arg)
            return -EFAULT;

        memset(&tmp, 0, sizeof(tmp));

        tmp.type        = tiny->serial.type;
        tmp.line        = tiny->serial.line;
        tmp.port        = tiny->serial.port;
        tmp.irq         = tiny->serial.irq;
        tmp.flags       = ASYNC_SKIP_TEST | ASYNC_AUTO_IRQ;
        tmp.xmit_fifo_size  = tiny->serial.xmit_fifo_size;
        tmp.baud_base       = tiny->serial.baud_base;
        tmp.close_delay     = 5*HZ;
        tmp.closing_wait    = 30*HZ;
        tmp.custom_divisor  = tiny->serial.custom_divisor;
        tmp.hub6        = tiny->serial.hub6;
        tmp.io_type     = tiny->serial.io_type;

        if (copy_to_user((void __user *)arg, &tmp, sizeof(struct serial_struct)))
            return -EFAULT;
        return 0;
    }
    return -ENOIOCTLCMD;
}

static int tiny_ioctl_tiocmiwait(struct tty_struct *tty, unsigned int cmd,
              unsigned long arg)
{
    struct tiny_serial *tiny = tty->driver_data;

    if (cmd == TIOCMIWAIT) {
        DECLARE_WAITQUEUE(wait, current);
        struct async_icount cnow;
        struct async_icount cprev;

        cprev = tiny->icount;
        while (1) {
            add_wait_queue(&tiny->wait, &wait);
            set_current_state(TASK_INTERRUPTIBLE);
            schedule();
            remove_wait_queue(&tiny->wait, &wait);

            /* see if a signal woke us up */
            if (signal_pending(current))
                return -ERESTARTSYS;

            cnow = tiny->icount;
            if (cnow.rng == cprev.rng && cnow.dsr == cprev.dsr &&
                cnow.dcd == cprev.dcd && cnow.cts == cprev.cts)
                return -EIO; /* no change => error */
            if (((arg & TIOCM_RNG) && (cnow.rng != cprev.rng)) ||
                ((arg & TIOCM_DSR) && (cnow.dsr != cprev.dsr)) ||
                ((arg & TIOCM_CD)  && (cnow.dcd != cprev.dcd)) ||
                ((arg & TIOCM_CTS) && (cnow.cts != cprev.cts))) {
                return 0;
            }
            cprev = cnow;
        }

    }
    return -ENOIOCTLCMD;
}

static int tiny_ioctl_tiocgicount(struct tty_struct *tty, unsigned int cmd,
              unsigned long arg)
{
    struct tiny_serial *tiny = tty->driver_data;

    if (cmd == TIOCGICOUNT) {
        struct async_icount cnow = tiny->icount;
        struct serial_icounter_struct icount;

        icount.cts  = cnow.cts;
        icount.dsr  = cnow.dsr;
        icount.rng  = cnow.rng;
        icount.dcd  = cnow.dcd;
        icount.rx   = cnow.rx;
        icount.tx   = cnow.tx;
        icount.frame    = cnow.frame;
        icount.overrun  = cnow.overrun;
        icount.parity   = cnow.parity;
        icount.brk  = cnow.brk;
        icount.buf_overrun = cnow.buf_overrun;

        if (copy_to_user((void __user *)arg, &icount, sizeof(icount)))
            return -EFAULT;
        return 0;
    }
    return -ENOIOCTLCMD;
}

/* the real tiny_ioctl function.  The above is done to get the small functions in the book */
static int tiny_ioctl(struct tty_struct *tty, unsigned int cmd,
              unsigned long arg)
{
    switch (cmd) {
    case TIOCGSERIAL:
        return tiny_ioctl_tiocgserial(tty, cmd, arg);
    case TIOCMIWAIT:
        return tiny_ioctl_tiocmiwait(tty, cmd, arg);
    case TIOCGICOUNT:
        return tiny_ioctl_tiocgicount(tty, cmd, arg);
    }

    return -ENOIOCTLCMD;
}


static const struct tty_operations serial_ops = {
    .open = tiny_open,
    .close = tiny_close,
    .write = tiny_write,
    .write_room = tiny_write_room,
    .set_termios = tiny_set_termios,
    .tiocmget = tiny_tiocmget,
    .tiocmset = tiny_tiocmset,
    .ioctl = tiny_ioctl,
};

static struct tty_driver *tiny_tty_driver;

int bl_tty_init(void)
{
    int retval;
    int i;

    /* allocate the tty driver */
    tiny_tty_driver = tty_alloc_driver(TINY_TTY_MINORS, 0);
    if (!tiny_tty_driver)
        return -ENOMEM;

    /* initialize the tty driver */
    tiny_tty_driver->owner = THIS_MODULE;
    tiny_tty_driver->driver_name = "blttysd";
    tiny_tty_driver->name = "blttysd";
    tiny_tty_driver->major = TINY_TTY_MAJOR,
    tiny_tty_driver->type = TTY_DRIVER_TYPE_SERIAL,
    tiny_tty_driver->subtype = SERIAL_TYPE_NORMAL,
    tiny_tty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV,
    tiny_tty_driver->init_termios = tty_std_termios;
    tiny_tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
    tty_set_operations(tiny_tty_driver, &serial_ops);
    for (i = 0; i < TINY_TTY_MINORS; i++) {
        tty_port_init(tiny_tty_port + i);
        tty_port_link_device(tiny_tty_port + i, tiny_tty_driver, i);
    }

    /* register the tty driver */
    retval = tty_register_driver(tiny_tty_driver);
    if (retval) {
        pr_err("failed to register tiny tty driver");
        tty_driver_kref_put(tiny_tty_driver);
        return retval;
    }

    for (i = 0; i < TINY_TTY_MINORS; ++i)
        tty_register_device(tiny_tty_driver, i, NULL);

    return retval;
}

void bl_tty_exit(void)
{
    struct tiny_serial *tiny;
    int i;

    for (i = 0; i < TINY_TTY_MINORS; ++i)
        tty_unregister_device(tiny_tty_driver, i);
    tty_unregister_driver(tiny_tty_driver);

    /* shut down all of the timers and free the memory */
    for (i = 0; i < TINY_TTY_MINORS; ++i) {
        tiny = tiny_table[i];
        if (tiny) {
            /* close the port */
            while (tiny->open_count)
                do_close(tiny);

            kfree(tiny);
            tiny_table[i] = NULL;
        }
    }
}

int bl_handle_ext_frame_tty(struct bl_eth_device *dev, const uint8_t *frame, const uint16_t frame_len)
{
    sdiowifi_ext_frame_tty_msg_t *msg;
    uint16_t tty_id;
    uint16_t tty_msg_len;
    struct tiny_serial *tiny;
    struct tty_struct *tty;
    struct tty_port *port;
    int i;
    int ret = 0;

    msg = (sdiowifi_ext_frame_tty_msg_t *)frame;
    if (!(frame_len > sizeof(*msg))) {
        ret = -1;
        goto exit;
    }
    tty_id = msg->tty_id;
    if (!(tty_id < TTY_DEV_COUNT)) {
        ret = -1;
        goto exit;
    }
    if (msg->type == SDIOWIFI_EXT_TTY_DATA) {
        tty_msg_len = frame_len - sizeof(*msg);

        tiny = tiny_table[tty_id];
        if (!tiny) {
            goto exit;
        }
        mutex_lock(&tiny->mutex);
        if (!(tiny->open_count > 0)) {
            goto exit_unlock;
        }

        tty = tiny->tty;
        port = tty->port;

        for (i = 0; i < tty_msg_len; ++i) {
            if (!tty_buffer_request_room(port, 1))
                tty_flip_buffer_push(port);
            tty_insert_flip_char(port, msg->data[i], TTY_NORMAL);
        }
        tty_flip_buffer_push(port);

exit_unlock:
        mutex_unlock(&tiny->mutex);
    } else if (msg->type == SDIOWIFI_EXT_TTY_BSR) {
        sdiowifi_ext_frame_tty_bsr_t *bsr = (sdiowifi_ext_frame_tty_bsr_t *)msg;
        if (bsr->flags & SDIOWIFI_EXT_TTY_BSR_INITIAL) {
            atomic_set(&dev->tty_credit, bsr->incr);
        } else {
            atomic_add(bsr->incr, &dev->tty_credit);
        }
        queue_work(dev->txworkqueue, &dev->tx_work);
    }
exit:
    return ret;
}
