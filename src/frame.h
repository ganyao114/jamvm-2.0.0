/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* Ensure operand stack is double-word aligned.  This leads to
   better double floating-point performance */
#define ALIGN_OSTACK(pntr) (uintptr_t*)(((uintptr_t)(pntr) + 7) & ~7)

//为方法执行创建栈帧
#define CREATE_TOP_FRAME(ee, class, mb, sp, ret)                \
{                                                               \
    /* 准备操作数栈 */                                            \
    Frame *last = ee->last_frame;                               \
    /* 计算新栈帧地址：                                             
    算法是新栈帧开始地址 = 上一个栈帧的开始地址 + 上个栈帧大小 */        \
    Frame *dummy = (Frame *)(last->ostack+last->mb->max_stack); \
    Frame *new_frame;                                           \
    uintptr_t *new_ostack;                                      \                                                            
    /* 新栈帧中本地变量栈开始地址 */                                \                                                            
    ret = (void*) (sp = (uintptr_t*)(dummy+1));                 \
    /* 新栈帧中操作数栈开始地址 */                                  \ 
    new_frame = (Frame *)(sp + mb->max_locals);                 \
    /* 对齐 */                                                  \
    new_ostack = ALIGN_OSTACK(new_frame + 1);                   \
                                                                \
    /* 检查栈溢出 */                                              \
    if((char*)(new_ostack + mb->max_stack) > ee->stack_end) {   \
        if(ee->overflow++) {                                    \
            /* Overflow when we're already throwing stack       \
               overflow.  Stack extension should be enough      \
               to throw exception, so something's seriously     \
               gone wrong - abort the VM! */                    \
            /* 如果走到这一步，意味着当前VM无法抛出栈溢出异常，
            因为只有嵌套抛出栈溢出
            即在处理栈溢出异常时依然栈溢出时候才会走到这里
            ，只能强制结束 VM */                                   \
            printf("Fatal stack overflow!  Aborting VM.\n");    \
            exitVM(1);                                          \
        }                                                       \
        ee->stack_end += STACK_RED_ZONE_SIZE;                   \
        signalException(java_lang_StackOverflowError, NULL);    \
        return NULL;                                            \
    }                                                           \
                                                                \
    dummy->mb = NULL;                                           \
    dummy->ostack = sp;                                         \
    dummy->prev = last;                                         \
                                                                \
    new_frame->mb = mb;                                         \
    new_frame->lvars = sp;                                      \
    new_frame->ostack = new_ostack;                             \
                                                                \
    new_frame->prev = dummy;                                    \
    ee->last_frame = new_frame;                                 \
}

#define POP_TOP_FRAME(ee)                                       \
    ee->last_frame = ee->last_frame->prev->prev;
