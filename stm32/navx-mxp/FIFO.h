/*
 * fifo.h
 *
 *  Created on: Jan 22, 2017
 *      Author: Scott
 */

#ifndef FIFO_H_
#define FIFO_H_

#define FIFO_NULL 0

/* FIFO is designed for one-writer, one reader use only.  Designed for use in
 * an embedded system where one client (either the writer or the reader) executes
 * within an interrupt service routine and the other in the foreground.
 *
 * The count variable is shared between the two clients and thus is incremented/
 * decrement using atomic operations.
 */
template <typename T, int N>
class FIFO
{
	T t[N];
	volatile int head;
	volatile int tail;
	volatile int count;

public:
	FIFO() {
		head = 0;
		tail = 0;
		count = 0;
	}

	bool is_full() { return count == N; }

	bool is_empty() { return count == 0; }

	int get_count() { return count; }

	T* enqueue_reserve() {
		if (count < N) {
			return &t[head];
		} else {
			return FIFO_NULL;
		}
    }

	bool enqueue_commit(T*p_t){
		if(p_t == &t[head]) {
			head++;
			if(head >= N) {
				head = 0;
			}
			/* Atomically increment the count */
			__sync_fetch_and_add(&count, 1);
			return true;
		}
		return false;
	}

    T* dequeue() {
		T* p_t;
    	if(count > 0) {
    		p_t = &t[tail];
		} else {
			p_t = FIFO_NULL;
		}
		return p_t;
    }

    bool dequeue_return(T* p_t) {
		if(p_t == &t[tail]) {
			tail++;
			if(tail >= N) {
				tail = 0;
			}
			/* Atomically decrement the count */
			__sync_fetch_and_sub(&count, 1);
			return true;
		}
		return false;
    }
};

#endif /* FIFO_H_ */
