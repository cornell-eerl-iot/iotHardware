#undef min
#undef max
#include <vector>

struct queue_t
{
  queue_t *next;
  std::vector<uint8_t> buffer; 
};


//Make the variables volatile since we want to change them during an interrupt.
queue_t *queue = nullptr;
queue_t *tail = nullptr;

void push_tail(queue_t *proc) {
	if (!queue) {
		queue = proc;
	}
	if (tail) {
		tail->next = proc;
	}
	tail = proc;
	proc->next = nullptr;
}

queue_t * pop_front_queue() {
	if (!queue) return nullptr;
	queue_t *proc = queue;
	queue = proc->next;
	if (tail == proc) {
		tail = nullptr;
	}
	proc->next = NULL;
	return proc;
}

namespace std {
  void __throw_length_error(char const*) {
  }
}

