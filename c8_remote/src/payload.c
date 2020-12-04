#include "tool/payload.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "tool/command.h"
#include "libpayload_int.h"

#include "dev/addr.h"

struct data
{
    DEV_PTR_T addr;
    int len;

    struct data *next;
    struct data *prev;
};

struct payload *dev_retrieve_payload(struct pwned_device *dev, PAYLOAD_T p)
{
    struct payload *curr;
    for(curr = dev->inst_pl; curr != NULL; curr = curr->next)
    {
        if(curr->type == p) return curr;
    }

    return NULL;
}

int dev_link_payload(struct pwned_device *dev, struct payload *pl)
{
    struct payload *curr;
    if(dev->inst_pl == NULL)
    {
        dev->inst_pl = pl;
        return CHECKM8_SUCCESS;
    }
    else
    {
        for(curr = dev->inst_pl; curr->next != NULL; curr = curr->next);

        curr->next = pl;
        pl->prev = curr;
        return CHECKM8_SUCCESS;
    }
}

int dev_unlink_payload(struct pwned_device *dev, struct payload *pl)
{
    if(dev->inst_pl == pl)
    {
        dev->inst_pl = pl->next;
        return CHECKM8_SUCCESS;
    }
    else
    {
        pl->prev->next = pl->next;
        if(pl->next != NULL)
            pl->next->prev = pl->prev;

        return CHECKM8_SUCCESS;
    }
}

struct data *dev_retrieve_data(struct pwned_device *dev, DEV_PTR_T addr)
{
    struct data *curr;
    for(curr = dev->inst_data; curr != NULL; curr = curr->next)
    {
        if(curr->addr == addr) return curr;
    }

    return NULL;
}

int dev_link_data(struct pwned_device *dev, struct data *data)
{
    struct data *curr;
    if(dev->inst_data == NULL)
    {
        dev->inst_data = data;
        return CHECKM8_SUCCESS;
    }
    else
    {
        for(curr = dev->inst_data; curr->next != NULL; curr = curr->next);

        curr->next = data;
        data->prev = curr;
        return CHECKM8_SUCCESS;
    }
}

int dev_unlink_data(struct pwned_device *dev, struct data *data)
{
    if(dev->inst_data == data)
    {
        dev->inst_data = data->next;
        return CHECKM8_SUCCESS;
    }
    else
    {
        data->prev->next = data->next;
        if(data->next != NULL)
            data->next->prev = data->prev;

        return CHECKM8_SUCCESS;
    }
}

int sync_payloads(struct pwned_device *dev)
{
    checkm8_debug_indent("sync_payloads(dev = %p)\n", dev);

    int ret, len, i;
    struct cmd_resp resp;
    struct install_args *installed;
    struct payload *pl;

    void *installed_buf;

    ret = open_device_session(dev);
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to open device session\n");
        return ret;
    }

    // 0xFFFF is the status command - first installed gadget in handler.c
    len = sizeof(struct cmd_resp) + NUM_GADGETS * sizeof(struct install_args);
    installed_buf = malloc(len);
    if(installed_buf == NULL)
    {
        checkm8_debug_indent("\tfailed to allocate response buffer\n");
        ret = -CHECKM8_FAIL_INT;
        goto close;
    }

    ret = command(dev, (short) 0xFFFF, NULL, 0, installed_buf, len);
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to get status of device\n");
        goto close;
    }

    memcpy(&resp, installed_buf, sizeof(struct cmd_resp));
    installed = (installed_buf + sizeof(struct cmd_resp));

    for(i = 0; i < resp.args[0]; i++)
    {
        // ignore builtins and empty slots
        if(installed[i].type == PAYLOAD_BUILTIN)
            continue;

        if(dev_retrieve_payload(dev, installed[i].type) != NULL)
        {
            checkm8_debug_indent("\tdoubly installed payload %i\n", installed[i].type);
            ret = -CHECKM8_FAIL_INVARGS;
            goto free;
        }

        pl = get_payload(installed[i].type);
        if(pl == NULL)
        {
            checkm8_debug_indent("\tgot unknown payload %i\n", installed[i].type);
            ret = -CHECKM8_FAIL_NOINST;
            goto free;
        }

        pl->intf = i;
        pl->install_base = (uint64_t) installed[i].addr;
        dev_link_payload(dev, pl);

        // data gadget is installed, lets sync this too
        if(installed[i].type == PAYLOAD_DATA)
            sync_data(dev);
    }

    ret = CHECKM8_SUCCESS;
free:
    free(installed_buf);

close:
    close_device_session(dev);
    return ret;
}

int sync_data(struct pwned_device *dev)
{
    checkm8_debug_indent("sync_data(dev = %p)\n", dev);

    int ret, len, i;
    struct cmd_resp resp;
    struct install_args *installed;
    struct data_args args = {
            .cmd = DATA_STATUS,
            .addr = 0,
            .len = 0
    };
    struct data *data;

    void *installed_buf;

    ret = execute_payload(dev, PAYLOAD_DATA,
                          &args, sizeof(struct data_args),
                          &resp, sizeof(struct cmd_resp));
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to get status of device\n");
        return ret;
    }

    len = sizeof(struct cmd_resp) + resp.args[0] * sizeof(struct install_args);
    installed_buf = malloc(len);
    if(installed_buf == NULL)
    {
        checkm8_debug_indent("\tfailed to allocate response buffer\n");
        ret = -CHECKM8_FAIL_INT;
        goto out;
    }

    ret = execute_payload(dev, PAYLOAD_DATA,
                          &args, sizeof(struct data_args),
                          installed_buf, len);
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to get installed addresses\n");
        goto free;
    }

    installed = (installed_buf + sizeof(struct cmd_resp));
    for(i = 0; i < resp.args[0]; i++)
    {
        if(dev_retrieve_data(dev, installed[i].addr) != NULL)
        {
            checkm8_debug_indent("\tdoubly installed data at %llx\n", installed[i].addr);
            goto free;
        }

        data = malloc(sizeof(struct data));
        if(data == NULL)
        {
            checkm8_debug_indent("\tfailed to allocate data struct\n");
            goto free;
        }

        data->addr = installed[i].addr;
        data->len = installed[i].len;
        dev_link_data(dev, data);
    }

free:
    free(installed_buf);
out:
    return ret;
}

int install_payload(struct pwned_device *dev, PAYLOAD_T p)
{
    checkm8_debug_indent("install_payload(dev = %p, p = %i)\n", dev, p);

    int ret, len;
    struct install_args args;
    struct cmd_resp resp;
    void *buf;
    struct payload *pl;

    if(dev_retrieve_payload(dev, p) != NULL)
    {
        checkm8_debug_indent("\tpayload installed already\n");
        return CHECKM8_SUCCESS;
    }

    pl = get_payload(p);
    if(pl == NULL)
    {
        checkm8_debug_indent("\tinvalid payload %i\n", p);
        return -CHECKM8_FAIL_INVARGS;
    }

    args.type = p;
    args.len = pl->len;
    args.addr = 0;

    len = sizeof(struct install_args) + pl->len;
    buf = malloc(len);
    if(buf == NULL)
    {
        checkm8_debug_indent("\tfailed to allocate transfer buffer\n");
        ret = -CHECKM8_FAIL_INT;
        goto exit;
    }

    memcpy(buf, &args, sizeof(struct install_args));
    memcpy(buf + sizeof(struct install_args), pl->data, pl->len);

    // 0xFFFE is the install command - second installed gadget in handler.c
    ret = command(dev, (short) 0xFFFE, buf, len, &resp, sizeof(struct cmd_resp));
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to execute gadget installer\n");
        goto free;
    }

    if(resp.status != CMD_SUCCESS)
    {
        checkm8_debug_indent("\tinstallation gadget returned %i\n", resp.status);
        ret = -CHECKM8_FAIL_NOINST;
        goto free;
    }

    checkm8_debug_indent("\tdone copying and linking payload\n");
    pl->intf = resp.args[0];
    pl->install_base = resp.args[1];
    dev_link_payload(dev, pl);

    ret = CHECKM8_SUCCESS;
free:
    free(buf);

exit:
    return ret;
}

int install_utils(struct pwned_device *dev)
{
    int ret, i;
    PAYLOAD_T utils[] = {PAYLOAD_READ, PAYLOAD_WRITE,
                         PAYLOAD_EXEC, PAYLOAD_DATA};

    for(i = 0; i < sizeof(utils) / sizeof(PAYLOAD_T); i++)
    {
        ret = install_payload(dev, utils[i]);
        if(IS_CHECKM8_FAIL(ret))
            return ret;
    }

    return CHECKM8_SUCCESS;
}


int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p)
{
    checkm8_debug_indent("uninstall_payload()\n");

    int ret;
    struct payload *inst, *pl;
    struct install_args args;
    struct cmd_resp resp;

    inst = dev_retrieve_payload(dev, p);
    if(inst == NULL)
    {
        checkm8_debug_indent("\tpayload is not installed, so not uninstalling\n");
        return CHECKM8_SUCCESS;
    }

    pl = get_payload(p);
    if(pl == NULL)
    {
        checkm8_debug_indent("\tinvalid payload %i\n", p);
        return -CHECKM8_FAIL_INVARGS;
    }

    if(inst->len != pl->len)
    {
        checkm8_debug_indent("\twarning: mismatch between installed and compiled lengths. removing old version.\n");
    }

    args.type = p;
    args.len = inst->len;
    args.addr = inst->install_base;

    ret = command(dev, (short) 0xFFFD,
                  &args, sizeof(struct install_args),
                  &resp, sizeof(struct cmd_resp));
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to execute gadget uninstaller\n");
        return ret;
    }

    if(resp.status != CMD_SUCCESS)
    {
        checkm8_debug_indent("\tuninstallation gadgedt returned %i\n", resp.status);
        return -CHECKM8_FAIL_NOTDONE;
    }

    dev_unlink_payload(dev, inst);
    free(inst);
    free(pl);
    return CHECKM8_SUCCESS;
}

int payload_is_installed(struct pwned_device *dev, PAYLOAD_T p)
{
    return dev_retrieve_payload(dev, p) != NULL;
}

int uninstall_all_payloads(struct pwned_device *dev)
{
    checkm8_debug_indent("uninstall_all_payloads(dev = %p)\n");
    int ret;
    while(dev->inst_pl != NULL)
    {
        ret = uninstall_payload(dev, dev->inst_pl->type);
        if(IS_CHECKM8_FAIL(ret))
        {
            checkm8_debug_indent("\terror while uninstalling\n");
            return ret;
        }
    }

    return CHECKM8_SUCCESS;
}

int execute_payload(struct pwned_device *dev, PAYLOAD_T p, void *arg, int arg_len, void *resp, int resp_len)
{
//    checkm8_debug_indent("execute_payload(dev = %p, p = %i, response_len = %i, nargs = %i, ...)\n",
//                         dev, p, response_len, nargs);
    struct payload *pl;

    if((pl = dev_retrieve_payload(dev, p)) == NULL)
    {
        checkm8_debug_indent("\tpayload is not installed\n");
        return -CHECKM8_FAIL_NOINST;
    }

    return command(dev, (short) (0xFFFF - pl->intf), arg, arg_len, resp, resp_len);
}

//
//int async_payload_create(struct pwned_device *dev, PAYLOAD_T p, DEV_PTR_T buf_addr)
//{
//    struct dev_cmd_resp *resp;
//    struct payload *pl;
//    unsigned long long task_args[5];
//
//    if((pl = dev_retrieve_payload(dev, p)) == NULL)
//    {
//        checkm8_debug_indent("\tpayload is not installed\n");
//        return CHECKM8_FAIL_NOINST;
//    }
//
//    task_args[0] = ADDR_TASK_NEW;
//    task_args[1] = 0x10001943b; // todo: name pointer
//    task_args[2] = pl->install_base;
//    task_args[3] = buf_addr;
//    task_args[4] = 0x4000; // todo: stack size
//
//    resp = dev_exec(dev, 0, 5, task_args);
//    if(IS_CHECKM8_FAIL(resp->ret))
//    {
//        checkm8_debug_indent("\tfailed to create a new task\n");
//        if(IS_CHECKM8_FAIL(free_address(dev, SRAM, buf_addr)))
//        {
//            checkm8_debug_indent("\talso failed to free buffer (something is really wrong)\n");
//        }
//
//        free_dev_cmd_resp(resp);
//        return CHECKM8_FAIL_XFER;
//    }
//
//    pl->async_base = resp->retval;
//    free_dev_cmd_resp(resp);
//    return CHECKM8_SUCCESS;
//}
//
//
//int async_payload_run(struct pwned_device *dev, PAYLOAD_T p)
//{
//    checkm8_debug_indent("run_payload_async(dev = %p, payload = %i)\n", dev, p);
//    struct payload *pl;
//    struct dev_cmd_resp *resp;
//    unsigned long long args[2];
//    int retval;
//
//    if((pl = dev_retrieve_payload(dev, p)) == NULL)
//    {
//        checkm8_debug_indent("\tpayload is not installed!\n");
//        return CHECKM8_FAIL_NOINST;
//    }
//
//    if(pl->async_base == DEV_PTR_NULL)
//    {
//        checkm8_debug_indent("\tasync payload is not set up correctly!\n");
//        return CHECKM8_FAIL_NOINST;
//    }
//
//    args[0] = ADDR_TASK_RUN;
//    args[1] = pl->async_base;
//
//    resp = dev_exec(dev, 0, 2, args);
//    retval = resp->ret;
//    free_dev_cmd_resp(resp);
//
//    return retval;
//}
//
//int async_payload_kill(struct pwned_device *dev, PAYLOAD_T p, DEV_PTR_T buf_addr)
//{
//    checkm8_debug_indent("kill_payload_async(dev = %p, p = %i, buf_addr = %llx)\n", dev, p, buf_addr);
//    struct payload *pl;
//    struct dev_cmd_resp *resp;
//    unsigned long long args[2];
//
//    if((pl = dev_retrieve_payload(dev, p)) == NULL)
//    {
//        checkm8_debug_indent("\tpayload is not installed\n");
//        return CHECKM8_FAIL_NOINST;
//    }
//
//    if(pl->async_base == DEV_PTR_NULL)
//    {
//        checkm8_debug_indent("\tasync payload is not set up correctly\n");
//        return CHECKM8_FAIL_NOINST;
//    }
//
//    args[0] = ADDR_TASK_FREE;
//    args[1] = pl->async_base;
//
//    resp = dev_exec(dev, 0, 2, args);
//    pl->async_base = DEV_PTR_NULL;
//
//    if(IS_CHECKM8_FAIL(resp->ret))
//    {
//        checkm8_debug_indent("\tfailed to kill payload\n");
//        free_dev_cmd_resp(resp);
//        return CHECKM8_FAIL_XFER;
//    }
//
//    free_dev_cmd_resp(resp);
////    if(IS_CHECKM8_FAIL(free_address(dev, SRAM, buf_addr)))
////    {
////        checkm8_debug_indent("\tfailed to free shared buffer\n");
////        return CHECKM8_FAIL_XFER;
////    }
//
//    return CHECKM8_SUCCESS;
//}
//
DEV_PTR_T install_data(struct pwned_device *dev, void *data, int len)
{
    checkm8_debug_indent("install_data(dev = %p, data = %p, len = %i)\n", dev, data, len);

    int ret, buflen;
    void *buf;

    struct cmd_resp resp;
    struct data *new_data;
    struct data_args args = {
            .cmd = DATA_INSTALL,
            .addr = 0,
            .len = 0
    };

    buflen = sizeof(struct data_args) + len;
    buf = malloc(buflen);
    if(buf == NULL)
    {
        checkm8_debug_indent("\tfailed to allocate transfer buffer\n");
        ret = -CHECKM8_FAIL_INT;
        goto exit;
    }

    memcpy(buf, &args, sizeof(struct data_args));
    memcpy(buf + sizeof(struct data_args), data, len);

    ret = execute_payload(dev, PAYLOAD_DATA, buf, buflen, &resp, sizeof(struct cmd_resp));
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to execute data installer\n");
        goto free;
    }

    if(resp.status != CMD_SUCCESS)
    {
        checkm8_debug_indent("\tinstallation gadget returned %i\n", resp.status);
        ret = -CHECKM8_FAIL_NOINST;
        goto free;
    }

    checkm8_debug_indent("\tdone transferring data\n");
    new_data = malloc(sizeof(struct data));
    if(new_data == NULL)
    {
        checkm8_debug_indent("\tfailed to allocate new data struct\n");
        ret = -CHECKM8_FAIL_INT;
        goto free;
    }

    new_data->addr = resp.args[0];
    new_data->len = len;
    dev_link_data(dev, new_data);

    ret = CHECKM8_SUCCESS;
free:
    free(buf);

exit:
    return ret;
}


int uninstall_data(struct pwned_device *dev, DEV_PTR_T addr)
{
    checkm8_debug_indent("uninstall_data(dev = %p, addr = %X)\n", dev, addr);

    int ret;
    struct data *data;
    struct cmd_resp resp;
    struct data_args args = {
            .cmd = DATA_UNINSTALL,
            .addr = addr,
            .len = 0
    };

    if((data = dev_retrieve_data(dev, addr)) == NULL)
    {
        checkm8_debug_indent("\tdata is not installed\n");
        return -CHECKM8_FAIL_NOINST;
    }

    ret = execute_payload(dev, PAYLOAD_DATA,
                          &args, sizeof(struct data_args),
                          &resp, sizeof(struct cmd_resp));
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to execute data uninstaller\n");
        return ret;
    }

    if(resp.status != CMD_SUCCESS)
    {
        checkm8_debug_indent("\tuninstaller gadget returned %i\n", resp.status);
        return -CHECKM8_FAIL_NOINST;
    }

    dev_unlink_data(dev, data);
    free(data);
    return CHECKM8_SUCCESS;
}

int uninstall_all_data(struct pwned_device *dev)
{
    checkm8_debug_indent("uninstall_all_data(dev = %p)\n", dev);
    int retval;

    while(dev->inst_data != NULL)
    {
        retval = uninstall_data(dev, dev->inst_data->addr);
        if(IS_CHECKM8_FAIL(retval))
        {
            checkm8_debug_indent("\terror while uninstalling data\n");
            return retval;
        }
    }

    return CHECKM8_SUCCESS;
}

int read_gadget(struct pwned_device *dev, DEV_PTR_T addr, void *dest, int len)
{
    checkm8_debug_indent("read_gadget(dev = %p, addr = %llx, len = %i)\n", dev, addr, len);

    int ret, buflen = sizeof(struct cmd_resp) + len;
    struct cmd_resp *resp;
    struct rw_args args = {
            .addr = addr,
            .len = len
    };

    void *resp_buf = malloc(buflen);
    if(resp_buf == NULL)
    {
        checkm8_debug_indent("\tfailed to allocate receive buffer\n");
        ret = -CHECKM8_FAIL_INT;
        goto out;
    }

    ret = execute_payload(dev, PAYLOAD_READ,
                          &args, sizeof(struct rw_args),
                          resp_buf, buflen);
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to execute read gadget\n");
        goto free;
    }

    resp = (struct cmd_resp *) resp_buf;
    if(resp->status != CMD_SUCCESS)
    {
        checkm8_debug_indent("\tread gadget returned %i\n", resp->status);
        ret = -CHECKM8_FAIL_NOINST;
        goto free;
    }

    memcpy(dest, resp_buf + sizeof(struct cmd_resp), len);
free:
    free(resp_buf);

out:
    return ret;
}

int write_gadget(struct pwned_device *dev, DEV_PTR_T addr, void *data, int len)
{
    checkm8_debug_indent("write_gadget(dev = %p, addr = %llx, data = %p, len = %i)\n", dev, addr, data, len);

    int ret, buflen = sizeof(struct rw_args) + len;
    struct cmd_resp resp;
    struct rw_args args = {
            .addr = addr,
            .len = len
    };

    void *send_buf = malloc(buflen);
    if(send_buf == NULL)
    {
        checkm8_debug_indent("\tfailed to allocate send buffer\n");
        ret = -CHECKM8_FAIL_INT;
        goto out;
    }

    memcpy(send_buf, &args, sizeof(struct rw_args));
    memcpy(send_buf + sizeof(struct rw_args), data, len);

    ret = execute_payload(dev, PAYLOAD_WRITE,
                          send_buf, buflen,
                          &resp, sizeof(struct cmd_resp));
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to execute write gadget\n");
        goto free;
    }

    if(resp.status != CMD_SUCCESS)
    {
        checkm8_debug_indent("\twrite gadget returned %i\n", resp.status);
        ret = -CHECKM8_FAIL_NOINST;
        goto free;
    }

    ret = CHECKM8_SUCCESS;
free:
    free(send_buf);

out:
    return ret;
}

int execute_gadget(struct pwned_device *dev, DEV_PTR_T addr, unsigned long long *retval, int nargs, ...)
{
    checkm8_debug_indent("execute_gadget(dev = %p, addr = %llx, nargs = %i)\n", dev, addr, nargs);
    int ret, i;
    struct exec_args args;
    struct cmd_resp resp;

    if(nargs > 8)
    {
        checkm8_debug_indent("\ttoo many arguments provided\n");
        ret = CHECKM8_FAIL_INVARGS;
        goto out;
    }

    va_list arg_list;
    va_start(arg_list, nargs);
    for(i = 0; i < nargs; i++)
    {
        args.args[i] = va_arg(arg_list, unsigned long long);
        checkm8_debug_indent("\textracted arg %lx\n", args.args[i]);
    }
    va_end(arg_list);

    ret = execute_payload(dev, PAYLOAD_EXEC,
                          &args, sizeof(struct exec_args),
                          &resp, sizeof(struct cmd_resp));
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to execute exec gadget\n");
        goto out;
    }

    if(resp.status != CMD_SUCCESS)
    {
        checkm8_debug_indent("\texec gadget returned %i\n", resp.status);
        ret = -CHECKM8_FAIL_NOINST;
        goto out;
    }

    *retval = resp.args[0];
    ret = CHECKM8_SUCCESS;

out:
    return ret;
}
//
//struct dev_cmd_resp *memset_gadget(struct pwned_device *dev, DEV_PTR_T addr, unsigned char c, int len)
//{
//    checkm8_debug_indent("memset_gadget(dev = %p, addr = %llx, c = %X, len = %i)\n", dev, addr, c, len);
//    return dev_memset(dev, addr, c, len);
//}
//
//struct dev_cmd_resp *memcpy_gadget(struct pwned_device *dev, DEV_PTR_T dest, DEV_PTR_T src, int len)
//{
//    checkm8_debug_indent("memcpy_gadget(dev = %p, dest = %llx, src = %llx, len = %i)\n", dev, dest, src, len);
//    return dev_memcpy(dev, dest, src, len);
//}