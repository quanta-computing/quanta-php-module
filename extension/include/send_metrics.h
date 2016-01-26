#ifndef SEND_METRICS_H_
#define SEND_METRICS_H_

#define BEGIN_OUTPUT_JSON_FORMAT \
"{\n"

#define PROFILING_OUTPUT_JSON_FORMAT \
"  \"profiling\": {\n\
    \"magento_loading\": %f,\n\
    \"before_layout_loading\": %f,\n\
    \"layout_loading\": %f,\n\
    \"between_layout_loading_and_rendering\": %f,\n\
    \"layout_rendering\": %f,\n\
    \"after_layout_rendering\": %f,\n\
    \"before_sending_response\": %f\n\
  },\n"

#define BLOCKS_BEGIN_OUTPUT_JSON_FORMAT \
"  \"blocks\": ["

#define BLOCK_OUTPUT_JSON_FORMAT \
"{\n\
    \"name\": \"%s\",\n\
    \"type\": \"%s\",\n\
    \"template\": \"%s\",\n\
    \"class\": \"%s\",\n\
    \"generate\": %f\n\
    \"renderize\": %f\n\
  }"

#define BLOCKS_END_OUTPUT_JSON_FORMAT \
"],\n"

#define END_OUTPUT_JSON_FORMAT \
"  \"flag\": %x\n\
}\n"


#define OUTBUF_QUANTA_SIZE 2048

#endif /* end of include guard: SEND_METRICS_H_ */
