/* SPDX-License-Identifier: GPL-2.0-only */

#include <linux/wait.h>

#define atomic_cond_read_relaxed(v, c)	smp_cond_load_relaxed(&(v)->counter, (c))

/**
 * clear_and_wake_up_bit - clear a bit and wake up anyone waiting on that bit
 *
 * @bit: the bit of the word being waited on
 * @word: the word being waited on, a kernel virtual address
 *
 * You can use this helper if bitflags are manipulated atomically rather than
 * non-atomically under a lock.
 */
static inline void clear_and_wake_up_bit(int bit, void *word)
{
	clear_bit_unlock(bit, word);
	/* See wake_up_bit() for which memory barrier you need to use. */
	smp_mb__after_atomic();
	wake_up_bit(word, bit);
}

/* bio stuffs */
#define REQ_OP_READ    READ
#define REQ_OP_WRITE   WRITE
#define bio_op(bio)    ((bio)->bi_rw & 1)

static inline void bio_set_op_attrs(struct bio *bio,
	unsigned op, unsigned op_flags) {
	bio->bi_rw = op | op_flags;
}

/* data.c */
static inline void __submit_bio(struct bio *bio, unsigned op, unsigned op_flags)
{
	bio_set_op_attrs(bio, op, op_flags);
	submit_bio(0, bio);
}

/* super.c */
static inline bool sb_rdonly(const struct super_block *sb) {
	return sb->s_flags & MS_RDONLY;
}

/**
 * smp_cond_load_relaxed() - (Spin) wait for cond with no ordering guarantees
 * @ptr: pointer to the variable to wait on
 * @cond: boolean expression to wait for
 *
 * Equivalent to using READ_ONCE() on the condition variable.
 *
 * Due to C lacking lambda expressions we load the value of *ptr into a
 * pre-named variable @VAL to be used in @cond.
 */
#ifndef smp_cond_load_relaxed
#define smp_cond_load_relaxed(ptr, cond_expr) ({		\
	typeof(ptr) __PTR = (ptr);				\
	typeof(*ptr) VAL;					\
	for (;;) {						\
		VAL = READ_ONCE(*__PTR);			\
		if (cond_expr)					\
			break;					\
		cpu_relax();					\
	}							\
	VAL;							\
})
#endif
