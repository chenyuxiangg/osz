# 说明

* Bxxx: Bug id
* Dxxx: 设计改进点id
* Qxxx: 待解决的问题（非bug）id

## 2025.11.8

1. [B001]`osz_msleep` 函数延时不准问题，比如 `osz_msleep(20)` 并不是真正的延时20ms;

> a. 已解决。-- 2025.11.12

2. [B002]任务运行结束return时无法正确处理逻辑，比如任务体return时，会进入到`inner_task_exit`函数，该函数处理完逻辑后是否应该调度？

> a. `inner_task_exit`中会调用`osz_task_delete`，该函数中会调度

## 2025.11.9

3. [B003]任务无法销毁

> a. 原因是传入`inner_task_exit`的参数(task_id)不正确，修改为获取当前任务的task_id -- 2025.11.12

5. 吐槽一下：AI写的ST是真的拉胯，还是得自己动手呀...

## 2025.11.12

6. [D001]任务栈应该设计为“自动申请”或者“用户手动传入”;
7. [D002]所有异常分支（直接return的地方）都应该增加调试手段，比如打印；

## 2025.11.13

8. [B004]带延时的`osz_sem_pend`函数处理有异常，函数状态会被置为0xc，且无法被加入就绪队列；

> a. 原因是待延时的`osz_sem_pend`函数既会将任务挂载到sem的pending列表，也会将任务挂载到sortlink的block列表。当超时唤醒时，由于pending链表和ready链表共用同一个结构体（互斥），且pending链表已经被挂载到sem上，所以无法将ready链表加入就绪队列，导致任务状态虽然被置为ready，但是无法被调度。-- 2025.11.14
> b. 该bug并未完全解决，当时仅考虑到超时后通过定时器唤醒的场景，并没有考虑`osz_sem_post`优先唤醒任务的场景，因此解决该问题的方法应该是将任务的状态置位为`TSK_FLAG_PENDING` | `TSK_FLAG_BLOCKING`。 -- 2025.11.15

## 2025.11.15

9. [B005]在osz_task_wake函数中增加一行printf打印后出现死机，问题所指的mepc为0；
    
> a. 原因是printf需要占用1024B的栈空间，当任务栈过小时，会导致越界飞踩，通过增大任务栈空间解决该问题；-- 2025.11.15

10.  [Q001]任务栈越界如何检测？场景：顺序越界踩 | 飞踩（比如使用prinf）

## 2025.11.16

11. [B006]为什么sem ST "Test_4_2" 的测试结果会与 "Test_4_1"有差异？

> a. 原因是Test_4_2中使用了`osz_sem_create`接口，接口并未对`osz_ipc_t`结构体首字段`supper`进行初始化，导致调用`osz_sem_pend`接口时返回`SEM_PEND_OBJ_NOT_SEM_ERR`从而跳过等待。-- 2025.11.17

## 2025.11.18

12. [B007]适配时钟后，`test_st_sem_g1.c`中的`Test_1_1: Single Task Semaphore Acquire and Release`用例概率性卡住，卡住时，app_main任务的状态一直为`RUNNING`。

> 该问题由两个异常引起： -- 2025.11.23
> a. 每个中断会执行`is_need_preemp`函数，该函数会从就绪队列取出一个就绪任务进行判定，当取出的任务不是当前任务时就调度，但这存在的问题是所有任务均会被抢占，而无关优先级；
> b. `osz_msleep`函数中主动调度的函数调用`os_schedule`未放置在关中断中，导致概率出现一种情况：当执行完`TASK_INT_UNLOCK(intSave);`后系统如果产生中断并调度走之后，如果该任务超时唤醒了，那它就会继续执行主动调度，而此时任务的状态是`RUNNING`态，`os_schedule`调度时并不会将当前任务置为`READY`态，因此导致当前任务脱离系统的管控，再也无法得到调度，下述为问题代码：

```c
void_t osz_msleep(uint64_t msec)
{
    uint32_t intSave = 0;
    uint64_t tick = (msec * SYSTEM_BASE_FREQ / MS_PER_SECOND) / CYCLES_PER_TICK;
    uint16_t task_id = osz_get_current_tid();
    TASK_INT_LOCK(&intSave);
    TASK_STATUS_CLEAN(task_id, TSK_FLAG_RUNNING);
    TASK_STATUS_SET(task_id, TSK_FLAG_BLOCKING);
    osz_sortlink_t *sl = STRUCT_ENTRY(osz_sortlink_t, list, TASK_LIST(task_id, blocking));
    if (inner_task_sortlink_is_mounted(task_id)) {
        inner_task_delete_sortlink(sl, task_id);
    }
    PSORTLINK_TIMEOUT(sl) = tick;
    inner_task_insert_sortlink(sl);
    TASK_INT_UNLOCK(intSave);
    os_schedule();              // 异常
}
```

## 2025.11.28

1.  [B008]`osz_events_read`函数在被唤醒场景下无法获取到读取到的事件，导致`outter_events`出参异常。

> a. 原因是`osz_events_read`中未在唤醒后对outter_events赋值。-- 2025.11.28

2. [Q002]如果task指定了`EVENT_FLAG_CLEAN`标志，那`osz_event_write`函数是否应该响应该标志？即是否该支持“写清”功能？如果没有写清功能，那事件只唤醒一个任务是否需要支持？

> a. 写清功能不符合实际场景，用户可能希望在响应事件后能够知晓是何种事件触发了任务唤醒，如果写清了，那么将导致唤醒后事件无法获取；
> b. 【WIP】事件只唤醒一个任务可以通过增加一个`EVENT_FLAG_WRITE_ONCE`标志，并在`osz_events_write`函数中增加相应逻辑实现。

3. [B009]写清功能导致事件读取时无法获取事件。

> a. 删除写清判断即可。-- 2025.11.28

4. [B010]当事件pend并被唤醒后，读清功能失效。

> a. 新增唤醒后读清功能。2025.11.28

## 2026.02.23

1. [B011]idle任务第一次被抢占后直接进入task exit函数，导致系统无背景线程，调度异常。

> a. g_is_positive_schechule被定义为BOOL，读取时只需要使用lb即可，使用lw会导致多读数据。