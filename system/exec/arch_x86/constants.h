#define BOOT_OFFSET 0x108000
#define AP_BOOT_OFFSET 0x8000
#define BOOT_STACK_SIZE 0x0400

#define ICU1 0x20    /* address of primary interrupt control unit */
#define ICU2 0xA0    /* address of slave interrupt control unit   */

#define OCR1 ICU1     /* Operation Command Register for ICU1      */
#define OCR2 ICU2     /* Operation Command Register for ICU2      */
#define IMR1 (ICU1+1) /* Interrupt Mask Register for ICU1         */
#define IMR2 (ICU2+1) /* Interrupt Mask Register for ICU2         */

#define EOI 0x20     /* end-of-interrupt signal                   */

