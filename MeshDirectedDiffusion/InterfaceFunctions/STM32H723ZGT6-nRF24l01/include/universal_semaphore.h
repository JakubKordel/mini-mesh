/**
 * @file universal_semaphore.h
 * @brief Universal semaphore interfaces for binary and counting semaphores.
 *
 * This header file provides a set of universal semaphore interfaces for both binary and counting
 * semaphores. These semaphore types can be used for synchronization and coordination in
 * multi-threaded applications. The implementation of these interfaces may vary depending on the
 * specific platform or environment.
 *
 */

#ifndef UNIVERSAL_SEMAPHORE_H_
#define UNIVERSAL_SEMAPHORE_H_

typedef struct binary_semaphore binary_semaphore_t;
typedef struct counting_semaphore counting_semaphore_t;

binary_semaphore_t* binary_semaphore_create(int initial_value);
void binary_semaphore_destroy(binary_semaphore_t* semaphore);
void binary_semaphore_wait(binary_semaphore_t* semaphore);
int binary_semaphore_wait_timeoutMS(binary_semaphore_t* semaphore, int timeout_ms);
void binary_semaphore_post(binary_semaphore_t* semaphore);

counting_semaphore_t* counting_semaphore_create(int initial_count);
void counting_semaphore_destroy(counting_semaphore_t* semaphore);
void counting_semaphore_wait(counting_semaphore_t* semaphore);
void counting_semaphore_post(counting_semaphore_t* semaphore);
void counting_semaphore_post_multiple(counting_semaphore_t* semaphore, int count);

#endif /* UNIVERSAL_SEMAPHORE_H_ */