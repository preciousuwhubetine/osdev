#include <multitasking.h>

using namespace crystalos;
using namespace crystalos::common;

TaskManager* TaskManager::kernelTasks = 0;

Task::Task(void entry())
{
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));

    cpustate->eax = 0;
    cpustate->ebx = 0;
    cpustate->ecx = 0;
    cpustate->edx = 0;

    cpustate->esi = 0;
    cpustate->edi = 0;
    cpustate->ebp = 0;

    cpustate->error = 0;
    cpustate->eip = (uint32_t)entry;
    cpustate->cs = 0x8;
    cpustate->eflags = 0x202;
    cpustate->esp = (uint32_t)stack;
    cpustate->ss = 0x10;
    
}

Task::~Task()
{

}

TaskManager::TaskManager()
{
    numTasks = 0;
    currentTask = 0;
    kernel_turn = false;
    if (kernelTasks == 0) kernelTasks = this;
}

TaskManager::~TaskManager()
{

}

bool TaskManager::AddTask(Task* task)
{
    if (numTasks > 255) return false;
    tasks[numTasks] = task;
    numTasks++;
    return true;
}

CPUState* TaskManager::Schedule(CPUState* cpustate)
{
    if (numTasks <= 0) return cpustate;

    if (kernel_turn)
    {
        tasks[currentTask]->cpustate = cpustate;
        currentTask++;
        if (currentTask >= numTasks) currentTask = 0;
        kernel_turn = false;
        return kernel_state;
    }
    else
    {
        kernel_state = cpustate;
        int tmp = currentTask;
        currentTask++;
        if (currentTask >= numTasks) currentTask = 0;
        kernel_turn = true;
        return tasks[tmp]->cpustate;
    }
}