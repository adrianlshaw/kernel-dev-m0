typedef struct task_t {
	void *entry_point;
	uint32_t r[13];
	uint32_t sp;
	uint32_t lr;
	uint32_t pc;
} task_t;
