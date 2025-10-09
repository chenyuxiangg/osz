# IPC

## 数据结构

```c
typedef struct __OSZ_LIST__ {
    void *next;
} osz_list_t;

typedef struct __OSZ_MODULE__ {
    uint32_t id;    // module id
    osz_list_t used_obj_list;
    osz_list_t free_obj_list; // free staitc obj list
} osz_module_t;

typedef struct __OSZ_OWNER__ {
    osz_taskcb_t *val;    // 考虑中断场景
    osz_list_t list;
} osz_owner_t;

typedef struct __OSZ_OBJ__ {
    list_t obj_list;    // 挂载到used_obj_list或者free_static_obj_list_head
    osz_owner_t owner;
    osz_module_t module;
    char name[OSZ_CFG_OBJ_NAME_MAX_LENS];
} osz_obj_t;

typedef struct __IPC_OBJ__ {
    osz_obj_t supper;
    struct {
        uint32_t   ipc_type : 16;
        uint32_t   ipc_create_type : 1;
        uint32_t   ipc_obj_used : 1;
        uint32_t   ipc_reserv : 14;
    } attr;
    union {
        uint32_t events;
        uint8_t  mutex;
        uint32_t sem;
        uint32_t *msg;
    } field;
    osz_list_t pend_list;
} osz_ipc_t;
```

## 事件集

### 事件集分类

事件集通过一个U32类型标识，每个bit表示一个事件。事件集是一种同步机制。OSZ中当前仅支持公有制事件。
* 公有制事件：每个bit表示一种唯一的事件，每个事件是整个系统共有的，而每个bit的解释权在消费者。因此使用者必须接受该事件可能由不符合其规定的生产者产生的非意愿事件。比如一种极端场景--由于事件bit资源是共享的，假设任务A在等待bit1，此时任务B也在等待bit1，极端情况下假设bit1对任务A、B的含义是互斥的，换句话说，对于任务A而言，bit1表示资源C充足，而对任务B而言表示资源C不足。任务A、B都需要在任务充足的情况下继续执行，这中情况下bit1就会存在冲突。

* 私有制事件：每个bit表示一种唯一的事件，每个事件的含义由该事件的生产者定义，事件由消费者订阅。

### 事件集使用场景

* 任务与任务通信
  该场景下所有任务均可执行read\write事件操作，需要注意任务等待不能形成互锁。

* 任务与中断通信
  该场景下仅有任务可以执行read事件操作，中断仅能执行write事件操作，write事件后在中断中处理等待任务的就绪状态。

* 一个任务等待一个事件
  任务仅等待事件集中的某个bit，任务等待某个bit置位。

* 一个任务等待多个事件
  任务等待事件集中的多个bit，当多个bit中的某个bit置位（OR），或者多个bit全部置位（AND）时任务唤醒。

* 多个任务等待一个事件
  多个任务等待在一个同一个事件集的同一bit时，系统默认所有等待者对该事件的含义解释都是相同的，事件通过等待列表组织等待者。

* 多个任务等待多个事件
  多个任务可以等待在同一个事件集的多个相同bit，也可以等待在多个不同事件集的不同bit。等待相同事件集的任务挂载在同一个事件等待队列里。

### 事件集支持的逻辑关系

* AND：所有事件同时发生，则唤醒任务
* OR：只要存在某一事件发生，则唤醒事件

### 事件集运行原理

系统可以创建多个事件集用于不同的目的，事件集可以表示历史上发生过某些事，当任务等待的所有事件在历史上都发生过时（AND关系下），任务得到唤醒；也可以表示此时此刻正在发生某件事（OR关系下）。任务会维护一个正在等待的事件列表以及等待的事件的关系，每当事件集中有新事件产生时，系统会遍历事件集等待列表，检查当前已产生的事件是否匹配每个正在等待的任务的事件列表。

事件集中清除事件bit没有多余的语义，如果某个任务需要监听某个事件清除，实际上了转换为“事件清除也是一个事件”，使用某一bit置位作为该语义即可。事件的清除有两个时机：读后清与写后清。读后清表示任务监听的所有事件发生时，唤醒任务后清除所有该任务等待的事件；写后清表示一组事件产生后，需要唤醒所有在该组事件上等待的任务后才清除本次产生的事件。前者适用于每次只唤醒一个任务，后者适用于唤醒所有等待的任务。**使用者需要特别关注读后清与写后清是否共存，共存时说明存在矛盾，任务有可能得不到唤醒。**

中断中不支持读事件集，仅支持写事件集。

### API接口

* `uint32_t osz_events_init(VOID)`: 初始化event资源，主要是初始化静态事件集的等待列表。返回初始化成功与否，初始化失败时需要通过模块异常信息`err_info`记录最近一次异常。
* `uint32_t osz_events_read(uint32_t event_set, uint8_t op_flag, uint32_t timeout)`: 调用者任务等待在event_set所标识的事件组上，当op_flag指定`EVENT_OP_CLEAN`时，任务唤醒后会清除event_set表示的事件位，当timeout到期时，调用者任务也会被唤醒。返回值用于表示错误原因或者唤醒原因。
* `uint32_t osz_events_write(osz_events_t *events, uint32_t event_set, uint8_t op_flag)`: 调用者任务将event_set写入事件集events，该函数中op_flag取值仅可以为`EVENT_OP_CLEAN`，表示写后清事件。返回值用于表示错误原因或者成功。
* `uint32_t osz_events_create(osz_events_t **out_ptr)`: 创建一个动态事件集，创建后的事件集通过out_ptr传出。返回值用于表示错误原因或者成功。
* `uint32_t osz_events_delete(osz_events_t *events)`: 删除一个动态事件集，删除时如果事件集中存在等待任务，则需全部唤醒，再执行删除操作。事件集删除后会将内存归还到内存池。返回值用于表示错误原因或者成功。
* `uint32_t osz_events_detach(osz_events_t *events)`: 解除一个静态事件集的使用，解除事件集时会将等待在该事件集上的任务全部唤醒。解除静态事件集后内存空间不会被清除。返回值用于表示错误原因或者成功。

### 测试用例

#### API测试

* fuzz测试<font color='red'>待定</font>;
* `osz_events_init`: 
  正常场景：初始化返回OSZ_OK，err_info为OSZ_EVENT_ERR_NO_ERR，初始化后所有静态事件集的等待队列为空。
  异常场景：NA
* `osz_events_read`: 
  正常场景：等待事件、事件触发唤醒、超时唤醒、timeout为0时直接返回
  异常场景：中断中调用事件等待，返回`EVENT_READ_IN_INT_ERR`
* `osz_events_write`: 
  正常场景：触发事件
  异常场景：