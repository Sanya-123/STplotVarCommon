#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "varcommon.h"


char* varloc_node_types[] = {
    "Base",
    "Struct",
    "Enum",
    "Pointer",
    "Union",
    "Array"
};


void for_each_var_loop(varloc_node_t* root, void(*func)(void*)){
    if (root == NULL){
        return;
    }
    if (root->child != NULL){
        for_each_var_loop(root->child, func);
    }
    if (root->next != NULL){
        for_each_var_loop(root->next, func);
    }
    func(root);
}

void varloc_delete_tree(varloc_node_t* root){
    if (root == NULL){
        return;
    }
    for_each_var_loop(root, free);
}


char* var_node_get_type_name(varloc_node_t* node){
    return varloc_node_types[node->var_type];
}

varloc_node_t* var_node_get_parent(varloc_node_t* child){
    if (child == NULL){
        return NULL;
    }
    while (child->parent == NULL){
        child = child->previous;
        if (child == NULL){
            return NULL;
        }
    }
    return child->parent;
}



int get_char_index(char* string, char c) {
    char *e = strchr(string, c);
    if (e == NULL) {
        return -1;
    }
    return (int)(e - string);
}


/*
 * Given variable name "a.b.c.d" composed of node names finds and
 * returns "d" node pointer or NULL if not found.
 * Any node in the link chain ("a", "b", "c") can be provided as root parameter
 *
 * Will also work if there are two nodes with the same name on one or multiple levels
 * (case of anonymous structs with "_")
 *
 * Creates one recursive call for each level ("." symbol in name)
 *
*/
varloc_node_t* var_node_get_by_name(varloc_node_t* root, char* name){
    int idx = get_char_index(name, '.');
    while(root != NULL){
        if (root->var_type == ARRAY){
            varloc_node_t* res = var_node_get_by_name(root->child, name);
            if (res != NULL){
                return res;
            }
        }
        if (idx < 0){
            if(strcmp(name, root->name) == 0){
                return root;
            }
        }
        else{
            if(strncmp(name, root->name, idx) == 0){
                varloc_node_t* res = var_node_get_by_name(root->child, name+idx+1);
                if (res != NULL){
                    return res;
                }
            }
        }
        root = root->next;
    }
    return NULL;
}


varloc_node_t* var_node_get_child_at_index(varloc_node_t* parent, uint32_t index){
    varloc_node_t* child = parent->child;
    for (uint_fast32_t i = 0; i < index; i++){
        if(child != NULL){
            child = child->next;
        }
        else{
            return NULL;
        }
    }
    return child;
}

int var_node_get_child_index(varloc_node_t* child){
    int row_n = 0;
    while(child->parent == NULL){
        child = child->previous;
        row_n++;
        if (child == NULL){
            //reach tree root
            return row_n;
        }
    }
    return row_n;
}

uint32_t var_node_get_address(varloc_node_t* node){
    uint32_t offset = node->address.base + (node->address.offset_bits / 8);
    varloc_node_t* parent = var_node_get_parent(node);
    while (parent != NULL){
        if (parent->var_type == POINTER){
            return 0xFFFFFFFF;
        }
        else if (parent->address.base == 0){
            offset += parent->address.offset_bits / 8;
            parent = var_node_get_parent(parent);
        }
        else{
            // top level variable with address
            offset += parent->address.base;
            return offset;
        }
    }
    return offset ;
}


varloc_location_t var_node_get_load_location(varloc_node_t* node){
    varloc_location_t loc = {0};
    if (node){
        varloc_node_t* parent = var_node_get_parent(node);
        if (!parent){
            parent = node;
        }
        uint32_t byte_address = var_node_get_address(parent);
        loc.address.base = (byte_address + (node->address.offset_bits / 8)) & 0xFFFFFFFC;
        loc.address.offset_bits = (((byte_address - loc.address.base) * 8) + node->address.offset_bits ) % 32;
        // loc.address.offset_bits = node->address.offset_bits % 32;
        loc.address.size_bits = node->address.size_bits;
        if (node->is_float){
            loc.type =  VARLOC_FLOAT;
        }
        else if (node->is_signed){
            loc.type = VARLOC_SIGNED;
        }

        loc.mask = (1 << loc.address.size_bits) - 1;
        if(loc.address.size_bits == 32)
            loc.mask = 0xFFFFFFFF;

        loc.mask = loc.mask << loc.address.offset_bits;
    }
    return loc;
}

varloc_node_t* new_var_node(){
    varloc_node_t* ret = malloc(sizeof(*ret));
    if (ret == NULL){
        printf("varloc_node_t malloc failed! exiting...\n");
        exit(1);
    }
    else{
        memset(ret, 0, sizeof(*ret));
    }
    return ret;
}

varloc_node_t* new_child(varloc_node_t* parent){
    varloc_node_t* child = new_var_node();
    // if parent already has child link to child next
    if (parent->child == NULL){
        parent->child = child;
        child->parent = parent;
    }
    else {
        varloc_node_t* node = parent->child;
        while(node->next != NULL){
            node = node->next;
        }
        node->next = child;
        child->previous = node;
    }
    return child;
}
