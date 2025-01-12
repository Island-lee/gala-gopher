/******************************************************************************
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * gala-gopher licensed under the Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Author: luzhihao
 * Create: 2022-11-25
 * Description: proc bpf prog
 ******************************************************************************/
#ifdef BPF_PROG_USER
#undef BPF_PROG_USER
#endif
#define BPF_PROG_KERN
#include "bpf.h"
#include "args_map.h"
#include "thread_map.h"
#include "proc_map.h"
#include "output_proc.h"

char g_linsence[] SEC("license") = "GPL";

#define BPF_F_INDEX_MASK    0xffffffffULL
#define BPF_F_ALL_CPU   BPF_F_INDEX_MASK

#ifndef __PERF_OUT_MAX
#define __PERF_OUT_MAX (64)
#endif
struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(u32));
    __uint(value_size, sizeof(u32));
    __uint(max_entries, __PERF_OUT_MAX);
} proc_exec_channel_map SEC(".maps");

#if (CURRENT_KERNEL_VERSION > KERNEL_VERSION(4, 18, 0))
KRAWTRACE(sched_process_exec, bpf_raw_tracepoint_args)
{
    struct proc_exec_evt event = {0};
    struct task_struct* task = (struct task_struct *)ctx->args[0];
    struct linux_binprm *bprm = (struct linux_binprm *)ctx->args[2];
    pid_t pid = _(task->pid);
    const char *filename = _(bprm->filename);

    event.pid = (u32)pid;
    bpf_probe_read(&event.filename, PATH_LEN, filename);

    bpf_perf_event_output(ctx, &proc_exec_channel_map, BPF_F_ALL_CPU,
                          &event, sizeof(event));
    return 0;
}
#else
SEC("tracepoint/sched/sched_process_exec")
int bpf_trace_sched_process_exec_func(struct trace_event_raw_sched_process_exec *ctx)
{
    struct proc_exec_evt event = {0};
    unsigned fname_off = ctx->__data_loc_filename & 0xFFFF;

    event.pid = bpf_get_current_pid_tgid() >> 32;
    bpf_probe_read_str(&event.filename, sizeof(event.filename), (void *)ctx + fname_off);

    bpf_perf_event_output(ctx, &proc_exec_channel_map, BPF_F_ALL_CPU,
                          &event, sizeof(event));
}
#endif
