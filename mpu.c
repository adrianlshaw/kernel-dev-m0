#include "main.h"

#define MPU_TYPE 0xE000ED90

uint32_t mpu_exists()
{
        return ioread32(MPU_TYPE);
}

uint32_t number_of_regions() {
	return (BIT_MASK(8,15) & mpu_exists()) >> 8;
}
