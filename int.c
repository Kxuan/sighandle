#define __GNU_SOURCE
#ifndef __x86_64
#error "You must run it in x86_64 platform!"
#endif

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include <unistd.h>

void sigfpe_action(int sig, siginfo_t *info, void *what)
{
    ucontext_t *u = (ucontext_t *) what;
    mcontext_t *mctx = &u->uc_mcontext;
    struct sigcontext *reg = (struct sigcontext *) mctx;
    //Decode cpu operator code
    if (0xf7 == *(unsigned char *) reg->rip)
    {
        unsigned char *op_ptr = (unsigned char *) reg->rip + 1;//next op
        if (0xe0 == (0xe0 & *op_ptr))
        {
            //Signed div instruction
            if (0x18 == (0x18 & *op_ptr))
            {
                //Using 64 bit registers
                switch (0x07 & *op_ptr)
                {
                    case 0://rax
                        reg->rax = 1;
                        break;
                    case 1://rcx
                        reg->rcx = 1;
                        break;
                    case 2://rdx
                        reg->rdx = 1;
                        break;
                    case 3://rbx
                        reg->rbx = 1;
                        break;
                    default:
                        //Unknown register
                        fprintf(stderr, "Fault: Divide by zero using register detected. But we cannot fix it.\n");
                        _exit(1);
                }
                printf("Warning: Divide by zero detected, the divisor has been set to 1. \n");

                return;
            }
        }
    }
    printf("Fault: SIGFPE detected but we cannot fix it. \n");
    _exit(1);
}

int main(int argc, char **argv)
{
    struct sigaction my_action;

    my_action.sa_sigaction = sigfpe_action;
    my_action.sa_flags = SA_SIGINFO;
    sigaction(SIGFPE, &my_action, NULL);

    printf(">>>Trying to calc 1.0 / 0.0\n");
    printf("float: 1.0 / 0.0 = %g\n", 1.0 / 0.0);
    printf(">>>Trying to calc 1 / 0\n");
    printf("int: 1 / 0 = %d\n", 1 / 0);
    printf(">>>Raise SIGFPE manual\n");
    raise(SIGFPE);
    return 0;
}
