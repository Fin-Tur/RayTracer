#pragma once
#include "../../rtweekend.h"
#include "../../hittables/hittable.h"
#include "../../hittables/hittable_list.h"
#include "aabb.h"


namespace bvh {

    struct node_4 {
        aabb content;
        uint32_t* objects;
        uint32_t obj_count = 0;
        node_4* n_left; //int left_child + flat list
        node_4* n_right;
        bool is_leaf = false;

        tree* tree;

        node_4(){
            n_left = nullptr;
            n_right = nullptr;
        }

        aabb get_aabb(){
            aabb box{point3{0, 0, 0}, point3{0, 0, 0}};
            for(int i = 0; i < obj_count; i++){
                box.include(tree->objects[objects[i]].get_aabb());
            }
            return box;
        }

        void free_node_4(){
            delete[] objects;
            delete(n_left);
            delete(n_right);
        }
    };

    struct tree {
        hittable* objects;
        uint32_t obj_count;
        node_4* root;

        
    };

    inline void bvh_tree_build_job(node_4* node){

        aabb curr_content = node->get_aabb();

        node->content = curr_content;

        if(node->obj_count <= 4){
            node->is_leaf = true;
            return;
        }

        vec3 extent = node->content.max - node->content.min;
        int axis = 0; //0 = x, 1 = y, 2 = z
        if (extent.y() > extent.x()) axis = 1;
        if (extent.z() > extent[axis]) axis = 2;

        //split lists
        uint32_t* h1_obs = new uint32_t[node->obj_count];
        uint32_t h1_obj_count = 0;
        uint32_t* h2_obs = new uint32_t[node->obj_count];
        uint32_t h2_obj_count = 0;


        const double split_pos = 0.5 * (node->content.min[axis] + node->content.max[axis]);

        for(int i = 0; i < node->obj_count; ++i){
            aabb ob = node->tree->objects[node->objects[i]].get_aabb();
            const double obj_center = 0.5 * (ob.min[axis] + ob.max[axis]);
            if (obj_center < split_pos) {
                h1_obs[h1_obj_count] = node->objects[i];
                h1_obj_count++;
            } else {
                h2_obs[h2_obj_count] = node->objects[i];
                h2_obj_count++;
            }
        }
        
        //create node and recursion
        node_4* n1 = new node_4();
        node_4* n2 = new node_4();
        node->n_left = n1;
        node->n_right = n2;
        n1->objects = h1_obs;
        n1->obj_count = h1_obj_count;
        n2->objects = h2_obs;
        n2->obj_count = h2_obj_count;

        if(h1_obj_count>0){
            bvh_tree_build_job(n1);
        }else{
            n1->free_node_4();
            delete(n1);
        }
        if(h2_obj_count>0){
            bvh_tree_build_job(n2);
        }else{
            n2->free_node_4();
            delete(n2);
        }
    }

    inline node_4* construct_bvh_tree(hittable* world, tree* tree) {
        hittable_list* list = dynamic_cast<hittable_list*>(world);
        tree->objects = *list->objects.data();
        tree->obj_count = list->objects.size();
        node_4* root = new node_4();
        tree->root = root;
        root->tree = tree;
        bvh_tree_build_job(root);
        return root;

    }

    inline void destroy_tree(node_4* root){
        if(root->n_left != nullptr){
            destroy_tree(root->n_left);
        }
        if(root->n_right != nullptr){
            destroy_tree(root->n_right);
        }
        delete[] root->objects;
        delete(root);
    }
    
}