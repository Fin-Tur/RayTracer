#pragma once
#include "../../rtweekend.h"
#include "../../hittables/hittable.h"
#include "../../hittables/hittable_list.h"
#include "aabb.h"


namespace bvh {

    struct node_4 {
        aabb content;
        uint8_t obj_start = 0;
        uint8_t obj_count = 0;
        node_4* n_left; //int left_child + flache liste
        node_4* n_right;
        bool is_leaf = false;

        node_4(){
            n_left = nullptr;
            n_right = nullptr;
        }
    };

    struct tree {
        hittable* objects;
        uint32_t obj_count;
        node_4* root;

        
    };

    inline void bvh_tree_build_job(tree* tree, node_4* node){

        aabb curr_content = list.get_aabb();
        node->content = curr_content;

        if(list.objects.size() <= 4){
            for(int i = 0; i < list.objects.size(); i++) { node->objects[i] = list.objects[i]; node->obj_count++;}
            node->is_leaf = true;
            return;
        }

        vec3 extent = node->content.max - node->content.min;
        int axis = 0; //0 = x, 1 = y, 2 = z
        if (extent.y() > extent.x()) axis = 1;
        if (extent.z() > extent[axis]) axis = 2;

        //split lists
        hittable_list h1;
        hittable_list h2;

        const double split_pos = 0.5 * (node->content.min[axis] + node->content.max[axis]);

        for(auto& obj : list.objects){
            aabb ob = obj->get_aabb();
            const double obj_center = 0.5 * (ob.min[axis] + ob.max[axis]);
            if (obj_center < split_pos) {
                h1.add(obj);
            } else {
                h2.add(obj);
            }
        }
        
        //create node and recursion
        node_4* n1 = new node_4();
        node_4* n2 = new node_4();
        node->n_left = n1;
        node->n_right = n2;
        if(h1.objects.size()>0){
            bvh_tree_build_job(h1, n1);
        }else{
            delete(n1);
        }
        if(h2.objects.size()>0){
            bvh_tree_build_job(h2, n2);
        }else{
            delete(n2);
        }
    }

    inline node_4* construct_bvh_tree(hittable* world, tree* tree) {
        hittable_list* list = dynamic_cast<hittable_list*>(world);
        tree->objects = *list->objects.data();
        tree->obj_count = list->objects.size();
        node_4* root = new node_4();
        tree->root = root;
        bvh_tree_build_job(tree, root);
        return root;

    }

    inline void destroy_tree(node_4* root){
        if(root->n_left != nullptr){
            destroy_tree(root->n_left);
        }
        if(root->n_right != nullptr){
            destroy_tree(root->n_right);
        }
        delete(root);
    }
    
}