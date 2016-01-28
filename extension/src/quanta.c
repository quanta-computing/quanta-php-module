#include "quanta_mon.h"

/**
 * Extract relevant informations in the _generateBlock parameters
 *
 * @author ch
 */
int qm_extract_param_generate_block(int i, zend_execute_data *execute_data TSRMLS_DC) {
  zval *param_node;
  const char *class_name;
  zend_uint class_name_len;
  zval *arr, *arr_final, **data, **data_final;
  HashTable *arr_hash, *arr_hash_final;
  HashPosition pointer, pointer_final;
  char *key, *tmpstrbuf;
  int key_len, typekey;
  long index;
  generate_renderize_block_details *current_gen_block_details;

  /* Do some sanity check, protect ourself against any modification of the magento core */
  /* For calling the debuger : __asm__("int3"); */
  if (!execute_data) {
    PRINTF_QUANTA("_generateBlock: execute_data NULL\n");
    return -1;
  }
  if (!execute_data->prev_execute_data) {
    PRINTF_QUANTA("_generateBlock: execute_data->prev_execute_data NULL\n");
    return -1;
  }
  if ((execute_data->prev_execute_data->function_state.arguments)[0] != (void*)2) {
    PRINTF_QUANTA("_generateBlock: execute_data->prev_execute_data->function_state.arguments is not 2 (%p)\n", \
    (execute_data->prev_execute_data->function_state.arguments)[0]);
    return -1;
  }
  param_node = (zval *)(execute_data->prev_execute_data->function_state.arguments)[-2];
  if (!param_node) {
    PRINTF_QUANTA ("_generateBlock: execute_data->prev_execute_data->function_state.arguments[-2] NULL\n");
    return -1;
  }

  if (Z_TYPE_P(param_node) != IS_OBJECT) {
    PRINTF_QUANTA ("_generateBlock: arguments[-2] is not an object\n");
    return -1;
  }

  if (!Z_OBJ_HANDLER(*param_node, get_class_name)) {
    PRINTF_QUANTA ("_generateBlock: arguments[-2] is an object of unknown class\n");
    return -1;
  }
  Z_OBJ_HANDLER(*param_node, get_class_name)(param_node, &class_name, &class_name_len, 0 TSRMLS_CC);
  if ((class_name_len != 30) && memcmp(class_name, "Mage_Core_Model_Layout_Element", 30)) {
    PRINTF_QUANTA ("_generateBlock: arguments[-1] is not a Mage_Core_Model_Layout_Element object (%s)\n", class_name);
    efree((char*)class_name);
    return -1;
  }
  efree((char*)class_name);

  /* Okay, we should have something that looks like what we want, now entering into the object
   * at first we have a single entry (@attributes) which is an array, grab the pointer and enumerate the array in the array */
  arr_hash = Z_OBJPROP_P(param_node);
  /* If we want to get a count : array_count = zend_hash_num_elements(arr_hash); */

  for (zend_hash_internal_pointer_reset_ex(arr_hash, &pointer);
       zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS;
       zend_hash_move_forward_ex(arr_hash, &pointer)) {

    if (zend_hash_get_current_key_ex(arr_hash, &key, &key_len, &index, 0, &pointer) == HASH_KEY_IS_STRING) {
      if ((key_len != 11) && memcmp(key, "@attributes", 11)) {
        #ifdef _QUANTA_MODULE_ULTRA_VERBOSE
          /* Sometimes we seen label, action, block */
          PRINTF_QUANTA ("_generateBlock: Unknown outer key '%s', skipping....\n", key);
        #endif
        continue;
      }
    } else {
      PRINTF_QUANTA ("_generateBlock: Hash key is not a string (%ld)\n", index);
      continue;
    }

    if (Z_TYPE_PP(data) != IS_ARRAY) {
      PRINTF_QUANTA ("_generateBlock: Object doesn't contain an array, skipping... (type=%d)\n", Z_TYPE_PP(data));
      continue;
    }
    current_gen_block_details = ecalloc(1, sizeof(generate_renderize_block_details));

    if (hp_globals.monitored_function_generate_renderize_block_first_linked_list == NULL)
      hp_globals.monitored_function_generate_renderize_block_first_linked_list = current_gen_block_details;
    else
      hp_globals.monitored_function_generate_renderize_block_last_linked_list->next_generate_renderize_block_detail = current_gen_block_details;

    hp_globals.monitored_function_generate_renderize_block_last_linked_list = current_gen_block_details;

    current_gen_block_details->tsc_generate_start = hp_globals.monitored_function_tsc_start[i];

    arr_hash_final = Z_ARRVAL_PP(data);
    for (zend_hash_internal_pointer_reset_ex(arr_hash_final, &pointer_final);
         zend_hash_get_current_data_ex(arr_hash_final, (void**) &data_final, &pointer_final) == SUCCESS;
         zend_hash_move_forward_ex(arr_hash_final, &pointer_final)) {

      if (Z_TYPE_PP(data_final) != IS_STRING) {
        PRINTF_QUANTA ("_generateBlock: Final array doesn't contain a string, skipping... (type=%d)\n", Z_TYPE_PP(data_final));
        continue;
      }
      if (zend_hash_get_current_key_ex(arr_hash_final, &key, &key_len, &index, 0, &pointer_final) != HASH_KEY_IS_STRING) {
        PRINTF_QUANTA ("_generateBlock: Hash final key is numeric (%ld) => %s\n", index, Z_STRVAL_PP(data_final));
        continue;
      }
      #ifdef _QUANTA_MODULE_ULTRA_VERBOSE
        PRINTF_QUANTA ("_generateBlock: final key '%s'=>'%s'\n", key, Z_STRVAL_PP(data_final)); //, Z_STRLEN_PP(data));
      #endif

      typekey = 0;
      switch(key_len) {
        case 5:
          if (!memcmp(key, "type", 4))
            typekey = 1;
          else if (!memcmp(key, "name", 4))
            typekey = 2;
          break;
        case 6:
          if (!memcmp(key, "class", 5))
            typekey = 3;
          break;
        case 9:
          if (!memcmp(key, "template", 8))
            typekey = 4;
          break;
        case 12:
          if (!memcmp(key, "@attributes", 11))
            typekey = -1;
          break;
        default:
          break;
      }

      if (typekey == -1)
        continue;

      if (typekey == 0) {
        #ifdef _QUANTA_MODULE_ULTRA_VERBOSE
          /* We also see keys : as, translate, before, after, output */
          PRINTF_QUANTA ("_generateBlock: Unknown final key='%s'=>'%s', skipping....\n", key, Z_STRVAL_PP(data_final));
        #endif
        continue;
      }
      index = strlen (Z_STRVAL_PP(data_final));
      tmpstrbuf = emalloc (index + 1);

      memcpy (tmpstrbuf, Z_STRVAL_PP(data_final), index);
      tmpstrbuf[index] = 0;

      switch (typekey) {
        case 1:  current_gen_block_details->type = tmpstrbuf;  break;
        case 2:  current_gen_block_details->name = tmpstrbuf;  break;
        case 3:  current_gen_block_details->class = tmpstrbuf;  break;
        case 4:  current_gen_block_details->template = tmpstrbuf;  break;
        default: PRINTF_QUANTA ("_generateBlock: ** BAD ** Unknown typekey (%d)\n", typekey); break;
      }
    } /* for - inner data */
  } /* for - outer data */
  return i;
}

int qm_extract_this_after_tohtml_do(char *name_in_layout) {
  generate_renderize_block_details *current_render_block_details;

  if (hp_globals.renderize_block_last_used) {
    /* Check if the last linked list is still the correct one */
    if (!strcmp(hp_globals.renderize_block_last_used->name, name_in_layout)) {
      /* Yes, we can use the shortcut */
      #ifdef _QUANTA_MODULE_ULTRA_VERBOSE
        PRINTF_QUANTA ("_afterToHtml:  Block name '%s' was just processed by _beforeToHtml. Using shortcut.\n", name_in_layout);
      #endif
      hp_globals.renderize_block_last_used->tsc_renderize_last_stop = cycle_timer();
      return 1;
    }
  }
  /* No, we are in a recursive call, we have to fallback to linked-list lookup */
  current_render_block_details = hp_globals.monitored_function_generate_renderize_block_first_linked_list;

  while (current_render_block_details) {
    if (!strcmp(current_render_block_details->name, name_in_layout))
      break;
    current_render_block_details = current_render_block_details->next_generate_renderize_block_detail;
  }
  if (!current_render_block_details) {
    /* Sometimes we get ANONYMOUS_xx blocks, ignore them */
    if (strlen(name_in_layout) > 10 && !strncmp(name_in_layout, "ANONYMOUS_", 10))
      return 0;
    PRINTF_QUANTA ("_afterToHtml:  Block name '%s' just finished the renderize phase but was not seen in the _generateBlock phase.\n", name_in_layout);
    return 0;
  }
  #ifdef _QUANTA_MODULE_ULTRA_VERBOSE
    PRINTF_QUANTA ("_afterToHtml:  Block name '%s' was NOT the last block processed by _beforeToHtml. Using slow path code.\n", name_in_layout);
  #endif
  current_render_block_details->tsc_renderize_last_stop = cycle_timer();
  return 1;
}

int qm_extract_this_before_tohtml_do(const char *class_name, zend_uint class_name_len,
char *name_in_layout, char *template) {
  int linked_list_pos = 1;
  generate_renderize_block_details *current_render_block_details;

  /* Ok, we have something, search into our linked list for nameInLayout */
  current_render_block_details = hp_globals.monitored_function_generate_renderize_block_first_linked_list;

  linked_list_pos = 1;
  while (current_render_block_details) {
    if (!strcmp(current_render_block_details->name, name_in_layout)) {
      if (!current_render_block_details->class) {
        current_render_block_details->class = emalloc(class_name_len + 1);
        memcpy (current_render_block_details->class, class_name, class_name_len);
        current_render_block_details->class[class_name_len] = 0;
      }
      /* Usually already filled by _generateBlock, but sometimes it's missing */
      if ((!current_render_block_details->template) && (template))
        current_render_block_details->template = template;
      else
        efree(template);
      if (!current_render_block_details->tsc_renderize_first_start)
        current_render_block_details->tsc_renderize_first_start = cycle_timer();

      hp_globals.renderize_block_last_used = current_render_block_details;
      #ifdef _QUANTA_MODULE_ULTRA_VERBOSE
        PRINTF_QUANTA ("_beforeToHtml: Block name '%s' found in the _generateBlock list at position %d\n", name_in_layout, linked_list_pos);
      #endif
      return 1;
    }
    linked_list_pos++;
    current_render_block_details = current_render_block_details->next_generate_renderize_block_detail;
  }
  /* Sometimes we get ANONYMOUS_xx blocks, ignore them */
  if (strlen(name_in_layout) > 10 && !strncmp(name_in_layout, "ANONYMOUS_", 10))
    return 0;
  PRINTF_QUANTA ("_beforeToHtml: Trying to renderize block %s not seen in _generateBlock. Ignoring.\n", name_in_layout);
  if (template)
   efree(template);
  return 0;
}

int qm_extract_this_before_after_tohtml(int is_after_tohtml, zend_execute_data *execute_data TSRMLS_DC) {
  zval *param_node, **data;
  const char *class_name;
  zend_uint class_name_len;
  HashTable *arr_hash;
  HashPosition pointer, pointer_final;
  char *key, *tmpstrbuf, *name_in_layout, *template = NULL;
  int key_len, typekey, i, ret;
  long index;

  /* Do some sanity check, protect ourself against any modification of the magento core */
  /* For calling the debuger : __asm__("int3"); */
  if (!execute_data) {
    PRINTF_QUANTA ("before/afterToHtml: execute_data NULL\n");
    return -1;
  }
  if (!execute_data->prev_execute_data) {
    PRINTF_QUANTA ("before/afterToHtml: execute_data->prev_execute_data NULL\n");
    return -1;
  }
  param_node = execute_data->prev_execute_data->current_this;
  if (Z_TYPE_P(param_node) != IS_OBJECT) {
    PRINTF_QUANTA ("before/afterToHtml: 'this' is not an object\n");
    return -1;
  }
  if (!Z_OBJ_HANDLER(*param_node, get_class_name)) {
    PRINTF_QUANTA ("before/afterToHtml: 'this' is an object of unknown class\n");
    return -1;
  }
  Z_OBJ_HANDLER(*param_node, get_class_name)(param_node, &class_name, &class_name_len, 0 TSRMLS_CC);
  #ifdef _QUANTA_MODULE_ULTRA_VERBOSE
    PRINTF_QUANTA ("before/afterToHtml: 'this' class=%s\n", class_name);
  #endif

  arr_hash = Z_OBJPROP_P(param_node);
  /* If we want to get a count : array_count = zend_hash_num_elements(arr_hash); */

  typekey = 0;

  for (zend_hash_internal_pointer_reset_ex(arr_hash, &pointer)
  ; zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS
  ; zend_hash_move_forward_ex(arr_hash, &pointer)) {

    int current_type_key = 0;
    if (zend_hash_get_current_key_ex(arr_hash, &key, &key_len, &index, 0, &pointer) != HASH_KEY_IS_STRING) {
      PRINTF_QUANTA ("before/afterToHtml: Hash key 'this' is not string (%ld)\n", index);
      continue;
    }
    if ((key_len == 17) && !memcmp(key, "\0*\0_nameInLayout", 17)) {
      typekey |= 0x1;
      current_type_key = 1;
    } else if ((key_len == 13) && !memcmp(key, "\0*\0_template", 13)) {
      if (Z_TYPE_PP(data) == IS_NULL)
        continue;
      typekey |= 0x2;
      current_type_key = 2;
    } else {
      #ifdef _QUANTA_MODULE_ULTRA_VERBOSE
      /* Enable this code for dumping members name of the current class */
        if (key_len > 3)
          PRINTF_QUANTA ("before/afterToHtml: Ignoring 'this' key+3 '%s' of len %d\n", key+3, key_len);
        else
          PRINTF_QUANTA ("before/afterToHtml: Ignoring 'this' key of len %d\n", key_len);
      #endif
      continue;
    }

    if (Z_TYPE_PP(data) != IS_STRING) {
      PRINTF_QUANTA("before/afterToHtml: Key %s has a value which is not a string (%d)\n", key+3, Z_TYPE_PP(data));
      continue;
    }
    #ifdef _QUANTA_MODULE_ULTRA_VERBOSE
      PRINTF_QUANTA "before/afterToHtml: typekey=%d value=%s\n", typekey, Z_STRVAL_PP(data));
    #endif
    index = strlen(Z_STRVAL_PP(data));
    tmpstrbuf = emalloc(index + 1);
    memcpy(tmpstrbuf, Z_STRVAL_PP(data), index);
    tmpstrbuf[index] = 0;
    if (current_type_key == 1) {
      name_in_layout = tmpstrbuf;
      if (is_after_tohtml)
        break; /* We don't need anything more (like template) here */
    } else {
      template = tmpstrbuf;
    }
  }
  if (typekey == 0x0) {
    PRINTF_QUANTA ("before/afterToHtml: 'this' doesn't have a member 'nameInLayout'\n");
    efree((char*)class_name);
    return i;
  } else if (typekey == 0x2) {
    PRINTF_QUANTA ("before/afterToHtml: 'this' doesnt have a member 'nameInLayout' but do have 'template'\n");
    efree(template);
    efree((char*)class_name);
    return i;
  }
  if (is_after_tohtml)
    ret = qm_extract_this_after_tohtml_do(name_in_layout);
  else
    ret = qm_extract_this_before_tohtml_do(class_name, class_name_len, name_in_layout, template);

  efree(name_in_layout);
  efree((char*)class_name);
  return i;
}

/**
 * Extract relevant informations in the _generateBlock parameters
 *
 * @author ch
 */
int qm_extract_this_before_tohtml(zend_execute_data *execute_data TSRMLS_DC) {
  return qm_extract_this_before_after_tohtml(0, execute_data);
}

int qm_after_tohmtl(zend_execute_data *execute_data TSRMLS_DC) {
  return qm_extract_this_before_after_tohtml(1, execute_data);
}

/**
 * Check if this entry should be ignored, first with a conservative Bloomish
 * filter then with an exact check against the function names.
 *
 * @author ch
 * return -1 if we don't monitor specifically this function, -2 if we don't monitor at all
 */
int qm_record_timers_loading_time(uint8_t hash_code, char *curr_func, zend_execute_data *execute_data TSRMLS_DC) {
  int i;

  /* Search quickly if we may have a match */
  if (!hp_monitored_functions_filter_collision(hash_code))
    return -1;

  if (hp_globals.profiler_level == QUANTA_MON_MODE_EVENTS_ONLY)
    i = POS_ENTRY_EVENTS_ONLY;
  else
    i = 0;

  /* We MAY have a match, enumerate the function array for an exact match */
  for (; hp_globals.monitored_function_names[i] != NULL; i++) {
    char *name = hp_globals.monitored_function_names[i];
    if (!strcmp(curr_func, name))
      break;
  }

  if (hp_globals.monitored_function_names[i] == NULL)
    return -1; /* False match, we have nothing */

  hp_globals.monitored_function_tsc_start[i] = cycle_timer();

  if (i == POS_ENTRY_BEFORETOHTML)
    return qm_extract_this_before_tohtml(execute_data);

  /* Something special for this function ? */
  if (i == POS_ENTRY_GENERATEBLOCK)
    return qm_extract_param_generate_block(i, execute_data);
  return i; /* No, bailout */
}
