#pragma once

#include "position.h"
/**
 * Position transition type.
 */

typedef struct _cortex_transition {
    char movestr[6];
    cortex_position* result;
} cortex_transition;

typedef struct _cortex_transition_entry {
    cortex_transition transition;
    struct _cortex_transition_entry* next;
} cortex_transition_entry;

cortex_transition_entry* cortex_transition_list_generate_legal(cortex_position* pos);
cortex_transition_entry* cortex_transition_list_free(cortex_transition_entry* list);

void cortex_transition_entry_free(cortex_transition_entry* entry);
