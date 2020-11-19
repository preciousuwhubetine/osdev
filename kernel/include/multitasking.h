#ifndef CRYSTALOS__MULTITASKING_H
#define CRYSTALOS__MULTITASKING_H

#include <common/types.h>
#include <IO/interrupts.h>

namespace crystalos
{
    struct CPUState
    {
        common::uint32_t eax;
        common::uint32_t ebx;
        common::uint32_t ecx;
        common::uint32_t edx;

        common::uint32_t esi;
        common::uint32_t edi;
        common::uint32_t ebp;

        common::uint32_t error;

        common::uint32_t eip;
        common::uint32_t cs;
        common::uint32_t eflags;
        common::uint32_t esp;
        common::uint32_t ss;
    }__attribute__((packed));

    class Task
    {
        friend class TaskManager;
        private:
            common::uint8_t stack[4096];
            CPUState* cpustate;

        public:
            Task(void entry());
            ~Task();
    };

    class TaskManager
    {
        private:    
            bool kernel_turn;
            Task* tasks[256];
            int numTasks;
            int currentTask;
            CPUState* kernel_state;
        
        public:
            TaskManager();
            ~TaskManager();

            static TaskManager* kernelTasks;

            bool AddTask(Task* task);
            CPUState* Schedule(CPUState* cpustate);
    };

}


#endif