


#define set_DQ		(GPIOA->BSRR=GPIO_BSRR_BS0)
#define clear_DQ	(GPIOA->BSRR=GPIO_BSRR_BR0)
#define test_DQ	(GPIOA->IDR&GPIO_IDR_IDR0)

void Init_1Wire_HW(void);

char init_1wire(char data);
void write_byte_1wire(char data);
char read_byte_1wire(void);



