/*
一、什么是cpu亲和性（affinity）
　　CPU的亲和性， 就是进程要在指定的 CPU 上尽量长时间地运行而不被迁移到其他处理器，也称为CPU关联性；
再简单的点的描述就将制定的进程或线程绑定到相应的cpu上；在多核运行的机器上，每个CPU本身自己会有缓存，缓存着进程使用的信息，而进程可能会被OS调度到其他CPU上，
如此，CPU cache命中率就低了，当绑定CPU后，程序就会一直在指定的cpu跑，不会由操作系统调度到其他CPU上，性能有一定的提高。
　　软亲和性（affinity）:  就是进程要在指定的 CPU 上尽量长时间地运行而不被迁移到其他处理器，Linux 内核进程调度器天生就具有被称为 软 CPU 亲和性（affinity） 的特性，
这意味着进程通常不会在处理器之间频繁迁移。这种状态正是我们希望的，因为进程迁移的频率小就意味着产生的负载小。
　　硬亲和性（affinity）：简单来说就是利用linux内核提供给用户的API，强行将进程或者线程绑定到某一个指定的cpu核运行。
　　解释：在linux内核中，所有的进程都有一个相关的数据结构，称为 task_struct。这个结构非常重要，原因有很多；
其中与 亲和性（affinity）相关度最高的是 cpus_allowed 位掩码。这个位掩码由 n 位组成，与系统中的 n 个逻辑处理器一一对应。 
具有 4 个物理 CPU 的系统可以有 4 位。如果这些 CPU 都启用了超线程，那么这个系统就有一个 8 位的位掩码。 如果为给定的进程设置了给定的位，那么这个进程就可以在相关的 CPU 上运行。
因此，如果一个进程可以在任何 CPU 上运行，并且能够根据需要在处理器之间进行迁移，那么位掩码就全是 1。实际上，这就是 Linux 中进程的缺省状态；


cpus_allowed用于控制进程可以在哪里处理器上运行

sched_set_affinity() （用来修改位掩码）
sched_get_affinity() （用来查看当前的位掩码）

　sched_setaffinity可以将某个进程绑定到一个特定的CPU。
你比操作系统更了解自己的程序，为了避免调度器愚蠢的调度你的程序，或是为了在多线程程序中避免缓存失效造成的开销，你可能会希望这样做


int sched_setaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask);
   该函数设置进程为pid的这个进程,让它运行在mask所设定的CPU上.如果pid的值为0,
   则表示指定的是当前进程,使当前进程运行在mask所设定的那些CPU上.
   第二个参数cpusetsize是mask所指定的数的长度.通常设定为sizeof(cpu_set_t).
   如果当前pid所指定的进程此时没有运行在mask所指定的任意一个CPU上,
   则该指定的进程会从其它CPU上迁移到mask的指定的一个CPU上运行.
   int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask);
   该函数获得pid所指示的进程的CPU位掩码,并将该掩码返回到mask所指向的结构中.
   即获得指定pid当前可以运行在哪些CPU上.
   同样,如果pid的值为0.也表示的是当前进程

    设置cpu affinity还需要用到以下宏函数
    void CPU_ZERO (cpu_set_t *set)
    这个宏对 CPU 集 set 进行初始化，将其设置为空集
    void CPU_SET (int cpu, cpu_set_t *set)
    这个宏将 指定的 cpu 加入 CPU 集 set 中
    void CPU_CLR (int cpu, cpu_set_t *set)
    这个宏将 指定的 cpu 从 CPU 集 set 中删除
    int CPU_ISSET (int cpu, const cpu_set_t *set)
    如果 cpu 是 CPU 集 set 的一员，这个宏就返回一个非零值（true），否则就返回零（false）
*/
#include "ngx_affinity.hpp"
#include "ngx_func.hpp"
int ngx_setaffinity(pid_t pid, int icpu)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(icpu, &set);
    if (sched_setaffinity(pid, sizeof(cpu_set_t), &set) == -1)
    {
        ngx_log_stderr(0, "ngx_setaffinity: setaffinity failed, pid = %d", pid);
        return -1;
    }
    else
    {
        return 0;
    }
    

}