#ifndef QM_UTILS_H_
#define QM_UTILS_H_

// CPU
int restore_cpu_affinity(cpu_set_t * prev_mask);
int bind_to_cpu(uint32_t cpu_id);
double get_cpu_frequency();
void clear_frequencies();

// Timings
uint64_t cycle_timer();
float cpu_cycles_range_to_ms(float cpufreq, long long start, long long end);
float cpu_cycles_to_ms(float cpufreq, uint64_t count);
double get_us_from_tsc(uint64_t count, double cpu_frequency);
long get_us_interval(struct timeval *start, struct timeval *end);

//Compat
zval *zend_hash_find_compat(HashTable *ht, const char *key, size_t key_len);
zend_bool zend_hash_exists_compat(HashTable *ht, const char *key, size_t key_len);
zval *zend_read_property_compat(zend_class_entry *ce, zval *obj, const char *name);

// Safe helpers
int safe_call_function(char *function, zval *ret, int ret_type,
  size_t params_count, zval params[]);
int safe_call_method(zval *object, char *function, zval *ret, int ret_type,
  size_t params_count, zval params[]);
zval *safe_get_argument(zend_execute_data *ex, size_t num, int type);
zval *safe_get_constant(const char *name, int type);

// This helpers
zval *get_this(zend_execute_data *execute_data);
zval *get_prev_this(zend_execute_data *execute_data);

// Object helpers
const char *get_obj_class_name(zval *obj);
int safe_new(char *class, zval *ret, int params_count, zval params[]);
int safe_get_class_constant(char *class, char *name, zval *ret, int type);


uint8_t hp_inline_hash(const char *str);
const char *hp_get_base_filename(const char *filename);
char *hp_get_function_name(zend_execute_data *data);
char *hp_get_function_name_fast(zend_execute_data *execute_data);
const char *hp_get_class_name(zend_execute_data *data);


#endif /* end of include guard: QM_UTILS_H_ */
