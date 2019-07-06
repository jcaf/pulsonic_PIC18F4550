#ifndef MAIN_H_
#define	MAIN_H_

    #include <xc.h> // include processor files - each processor file is guarded.  
    #include <stdint.h>
    #define F_CPU 48000000UL//48MHz
    #include "system.h"
    #include "types.h"

    struct _main_flag
    {
        unsigned f1ms:1;
        unsigned __a:7;
    };
    extern volatile struct _main_flag main_flag;
    
    #define myitoa(_integer_, _buffer_, _base_) itoa(_buffer_, _integer_, _base_)

#ifdef	__cplusplus
    extern "C" {
    #endif 
    #ifdef	__cplusplus
    }
    #endif 

#endif	

