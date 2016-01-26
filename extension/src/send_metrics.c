#include "quanta_mon.h"
#include "send_metrics.h"

// returns ouput file FD or -1
static int init_output(char *bufout) {
  struct timeval tv;
  return gettimeofday(&tv, NULL) == -1 ||
    snprintf(bufout, OUTBUF_QUANTA_SIZE, "/tmp/quanta-%d.%d-%d.txt",
    tv.tv_sec, tv.tv_usec, getpid()) < 4 ? -1 :
    open(bufout, O_WRONLY | O_APPEND | O_CREAT, 0644);
}

// writes content into the output file and set the debug info if necessary
static void output(int size, char *bufout, int fd_log_out) {
  if (size > 0 && write(fd_log_out, bufout, size) < 0)
    hp_globals.quanta_dbg |= 0x10000;
}

// begins the output
static void begin_output(int fd_log_out) {
  output(strlen(BEGIN_OUTPUT_JSON_FORMAT),
    BEGIN_OUTPUT_JSON_FORMAT, fd_log_out);
}

// ouputs the profiling data
static void profiler_output(char *bufout, int fd_log_out, struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  long long *starts = hp_globals.monitored_function_tsc_start,
    *stops = hp_globals.monitored_function_tsc_stop;
  output(snprintf(bufout, OUTBUF_QUANTA_SIZE,
    PROFILING_OUTPUT_JSON_FORMAT,
    cpu_cycles_to_ms(cpufreq, starts[0], starts[1]),
    cpu_cycles_to_ms(cpufreq, starts[1], starts[2]),
    cpu_cycles_to_ms(cpufreq, starts[2], stops[2]),
    cpu_cycles_to_ms(cpufreq, stops[2], starts[3]),
    cpu_cycles_to_ms(cpufreq, starts[3], stops[3]),
    cpu_cycles_to_ms(cpufreq, stops[3], stops[4]),
    cpu_cycles_to_ms(cpufreq, stops[4], stops[5])),
  bufout, fd_log_out);
  monikor_metric_list_push(metrics, monikor_metric_float(
    "magento.loading_time", clock, cpu_cycles_to_ms(cpufreq, starts[0], starts[1]), 0)
  );
  monikor_metric_list_push(metrics, monikor_metric_float(
    "magento.layout_loading_time", clock, cpu_cycles_to_ms(cpufreq, starts[2], stops[2]), 0)
  );
  monikor_metric_list_push(metrics, monikor_metric_float(
    "magento.layout_rendering_time", clock, cpu_cycles_to_ms(cpufreq, starts[3], stops[3]), 0)
  );
}

// outputs a single block data
static void block_output(char *bufout, int fd_log_out,
float cpufreq, generate_renderize_block_details *block) {
  output(snprintf(bufout, OUTBUF_QUANTA_SIZE, BLOCK_OUTPUT_JSON_FORMAT,
    block->name, block->type, block->template, block->class,
    cpu_cycles_to_ms(cpufreq, block->tsc_generate_start, block->tsc_generate_stop),
    cpu_cycles_to_ms(cpufreq, block->tsc_renderize_first_start, block->tsc_renderize_last_stop))
  , bufout, fd_log_out);
  if (block->name) efree(block->name);
  if (block->type) efree(block->type);
  if (block->template) efree(block->template);
}

// outputs a blocks data
static void blocks_output(char *bufout, int fd_log_out, float cpufreq) {
  output(strlen(BLOCKS_BEGIN_OUTPUT_JSON_FORMAT),
    BLOCKS_BEGIN_OUTPUT_JSON_FORMAT, fd_log_out);
  generate_renderize_block_details *current_block, *prev_block;
  current_block = hp_globals.monitored_function_generate_renderize_block_first_linked_list;
  while (current_block) {
    block_output(bufout, fd_log_out, cpufreq, prev_block = current_block);
    current_block = current_block->next_generate_renderize_block_detail;
    if (current_block) output(2, ", ", fd_log_out);
    efree(prev_block);
  }
  output(strlen(BLOCKS_END_OUTPUT_JSON_FORMAT),
    BLOCKS_END_OUTPUT_JSON_FORMAT, fd_log_out);
}

// ends output
static void end_output(char *bufout, int fd_log_out) {
  output(snprintf(bufout, OUTBUF_QUANTA_SIZE, END_OUTPUT_JSON_FORMAT,
    hp_globals.quanta_dbg), bufout, fd_log_out);
}


static void send_data_to_monikor(monikor_metric_list_t *metrics) {
  void *data = NULL;
  int sock;
  size_t size;
  struct sockaddr_un addr;

  if (!hp_globals.path_quanta_agent_socket) {
    PRINTF_QUANTA("Cannot send data to monikor: socket not configured\n");
    return;
  }

  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, hp_globals.path_quanta_agent_socket);
  if (monikor_metric_list_serialize(metrics, &data, &size)) {
    PRINTF_QUANTA("Cannot send data to monikor: cannot serialize metrics\n");
    free(data);
    return;
  }
  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1
  || connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1
  || write(sock, data, size) == -1) {
    PRINTF_QUANTA("Cannot send data to monikor: %s\n", strerror(errno));
  }
  free(data);
  close(sock);
}
