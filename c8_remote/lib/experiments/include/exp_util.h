#ifndef CHECKM8_TOOL_EXPERIMENT_UTIL_H
#define CHECKM8_TOOL_EXPERIMENT_UTIL_H

struct thread_tracker;

struct thread_tracker *create_and_save(struct thread_tracker **head, void *(*func)(void *), void *arg);
void *get_arg(struct thread_tracker *tracker);
void set_finished(struct thread_tracker *tracker);


#endif //CHECKM8_TOOL_EXPERIMENT_UTIL_H
