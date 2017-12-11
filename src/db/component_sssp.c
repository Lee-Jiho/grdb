#include <fcntl.h>
#include <math.h>
#include <limits.h>
#include "graph.h"
#include "cli.h"
/* unneeded */
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <limits.h>
//#include <inttypes.h>


int component_sssp(component_t c,vertexid_t vertex_1,vertexid_t vertex_2,int *n,int *total_weight,vertexid_t **path){
	/*
	Previous implementation for creating and counting vertexes
	
	
	int createVertexIfNotContains(component_t c, vertexid_t id)
	{
    vertex_t current = c->v;
    vertex_t prev = NULL;
    
    while (current != NULL)
    {
        if (current->id == id)
        {
            return 0;
            
        }
        prev = current;
        current = current->next;
    }
    
    vertex_t v = malloc(sizeof(struct vertex));
    
    vertex_init(v);
    v->id = id;
    
    if (c->v == NULL)
    {
        c->v = v;
    }
    
    v->prev = prev;
    if (prev != NULL)
    {
        prev->next = v;
    }
    return 1;
}

int readInEdges(component_t c)
{
    char s[BUFSIZE];
    
    memset(s, 0, BUFSIZE);
    sprintf(s, "%s/%d/%d/e", grdbdir, gno, cno);
    c->efd = open(s, O_RDWR | O_CREAT, 0644);
    if (c->efd < 0) {
        printf("Open edge file failed\n");
        return -1;
    }
    
    off_t off;
    ssize_t len, size;
    char buf[sizeof (vertexid_t) << 1];
    
    if (c->se == NULL)
        size = 0;
    else
        size = schema_size(c->se);
    
    edge_t prev_e = NULL;
    
    int numVertices = 0;
    for (off = 0;; off += (sizeof (vertexid_t) << 1) + size) {
        lseek(c->efd, off, SEEK_SET);
        len = read(c->efd, buf, sizeof (vertexid_t) << 1);
        if (len != sizeof (vertexid_t) << 1) {
            break;
        }
        
        edge_t e = malloc(sizeof(struct edge));
        edge_init(e);
        
        e->id1 = *((vertexid_t *) buf);
        e->id2 = *((vertexid_t *) (buf + sizeof (vertexid_t)));

        numVertices += createVertexIfNotContains(c, e->id1);
        numVertices += createVertexIfNotContains(c, e->id2);

        if (e->tuple == NULL)
            tuple_init(&(e->tuple), c->se);

        memset(e->tuple->buf, 0, size);
        len = read(c->efd, e->tuple->buf, size);
        
        if (c->e == NULL) {
            c->e = e;
        }
        e->prev = prev_e;

        if (prev_e != NULL) {
            prev_e->next = e;
        }
        prev_e = e;
    }
    
    return numVertices;
}
	
	
	
	*/

	// finding the total amount of vertices in our graph
	int number_of_verticies(){

		off_t off;
		ssize_t len;
		char s[BUFSIZE], *buf;
		int fd, length, num_v;
		length = sizeof(vertex_t);
		num_v = 0;
		buf = malloc(length);

		// open file
		memset(s, 0, BUFSIZE);
		sprintf(s, "%s/%d/%d/v", grdbdir, gno, cno);
		fd = open(s, O_RDONLY);

		// read the file and keep count of # of verticies
		for (off = 0;;off += length){
			lseek(fd, off, SEEK_SET);
			len = read(fd, buf, length);
			if (len <= 0)
				break;
			num_v += 1;
		}

		// close file
		close(fd);

		return(num_v);
	}

	// insert new v in array
	void alter_path_array(vertexid_t **path, vertexid_t new, int *path_size){
		// reallocate memory
		vertexid_t *temp_path_for_update = realloc(*path, (*path_size + 1) * sizeof(vertexid_t));
		// keeping track of path size
		if (temp_path_for_update){
			temp_path_for_update[*path_size] = new + 1;
			*path = temp_path_for_update;
			*path_size += 1;
		}
	}

	// get shortest path by working backwards
	void find_the_path_backwards(int num_v, vertexid_t parent_array[num_v]){
		vertexid_t current, temp[num_v];
		int m = num_v - 1;

		// keeping track of where we are
		current = vertex_2;

		// temporary path
		for (int i = 0; i < num_v; i += 1){
			temp[i] = -1;
		}

		// create path
		while (parent_array[current] != -1){
			temp[m] = current;
			current = parent_array[current];
			m -= 1;
		}

		// update path array
		for (int i = m + 1; i < num_v; i += 1){
			alter_path_array(path, temp[i], n);
		}
	}

	// input the weight of the edges given a vertice via file
	int read_edge_weight_from_file(vertexid_t u, vertexid_t v) {
		char s[BUFSIZE];
		int fd, weight, offset;
		struct edge e;
		attribute_t attr;

		weight = -1;

		// open edge file
		memset(s, 0, BUFSIZE);
		sprintf(s, "%s/%d/%d/e", grdbdir, gno, cno);
		fd = open(s, O_RDONLY);

		// get edges
		edge_init(&e);
		edge_set_vertices(&e, u, v);
		edge_read(&e, c->se, fd);

		// find the edge weight that will be returned
		if (e.tuple != NULL){
			attr = e.tuple->s->attrlist;
			offset = tuple_get_offset(e.tuple, attr->name);
			if (offset >= 0){
				weight = tuple_get_int(e.tuple->buf + offset);
			}
		}

		// close file
		close(fd);

		// return weight
		return(weight);
	}

	// initalizing our shortest path array and tracking cost array
	// min_path_found_array will be our shortest path and current_distance_array tracks the cost
	void init_min_path_found_array_current_distance_array(int num_v, int min_path_found_array[num_v], int current_distance_array[num_v], vertexid_t parent_array[num_v], vertexid_t vertex_1){
		for (int i = 0; i < num_v; i += 1){
			if (i == (vertex_1)){
				min_path_found_array[i] = 1;
				current_distance_array[i] = 0;
			} else {
				min_path_found_array[i] = 0;
				current_distance_array[i] = INT_MAX;
			}
			parent_array[i] = -1;
		}
	}

	// intializing our cost matrix and adjacency matrix
	void init_adjacency_matrix_cost_matrix(int num_v, int adjacency_matrix[num_v][num_v], int cost_matrix[num_v][num_v]){
		int result;
		for (int i = 0; i < num_v; i += 1){
			for (int j = 0; j < num_v; j += 1){
				result = read_edge_weight_from_file(i + 1, j + 1);
				if (result >= 0){
					adjacency_matrix[i][j] = 1;
					cost_matrix[i][j] = result;
				} else {
					adjacency_matrix[i][j] = 0;
					cost_matrix[i][j] = INT_MAX;
				}
			}
		}
	}

	// checking if vertex v is in our shortest path
	int in_min_path_found_array(int num_v, int min_path_found_array[num_v], vertexid_t v){
		return min_path_found_array[v];
	}

	// checking vertices that have been added to our shortest path
	int bool_is_array_full(int num_v, int min_path_found_array[num_v]){
		int full = 1;
		for (int i = 0; i < num_v; i += 1){
			if (min_path_found_array[i] == 0)
				full = 0;
		}

		return full;
	}

	// find the cheapest edge that already isn't in the current shortest path
	vertexid_t choose_min_path(int num_v,int min_path_found_array[num_v],int adjacency_matrix[num_v][num_v],int cost_matrix[num_v][num_v]){

		int min_weight = INT_MAX;
		vertexid_t new_vert, parent;

		for (vertexid_t i = 0; i < num_v; i += 1){
			// if vertex i is in our shortest path
			// check cost of neighboring vertices
			if (min_path_found_array[i] == 1){   
				for (vertexid_t j = 0; j < num_v; j += 1){
					if (!in_min_path_found_array(num_v, min_path_found_array, j) && cost_matrix[i][j] < min_weight){
						min_weight = cost_matrix[i][j];
						parent = i;
						new_vert = j;
					}
				}
			}
		}

		min_path_found_array[new_vert] = 1;

		return new_vert;
	}
	
	// function to relax our edges
	void make_edges_relaxed(int num_v, vertexid_t w,
					 int adjacency_matrix[num_v][num_v],
					 int cost_matrix[num_v][num_v],
					 int current_distance_array[num_v],
					 vertexid_t parent_array[num_v])
	{
		for (int i = 0; i < num_v; i += 1){
			if (adjacency_matrix[w][i] && current_distance_array[i] > (current_distance_array[w] + cost_matrix[w][i])){
				current_distance_array[i] = current_distance_array[w] + cost_matrix[w][i];
				parent_array[i] = w;
			}
		}
	}

	vertex_1 -= 1;
	vertex_2 -= 1;
	int num_v = number_of_verticies();
	// parent array is the original list
	vertexid_t w, parent_array[num_v];   

	// min_path_found_array is the minimum path that was found
	// current_distance_array is the current length of the list
	int min_path_found_array[num_v], current_distance_array[num_v];

	int adjacency_matrix[num_v][num_v], cost_matrix[num_v][num_v]; 
	vertexid_t *temp_path_for_update = malloc(sizeof(vertexid_t));


	// intialize path
	if (temp_path_for_update){
		*path = temp_path_for_update;
		*path[0] = vertex_1 + 1;
	}
	*n = 1;

	// initialize structures
	init_min_path_found_array_current_distance_array(num_v, min_path_found_array, current_distance_array, parent_array, vertex_1);
	init_adjacency_matrix_cost_matrix(num_v, adjacency_matrix, cost_matrix);

	// relax the edges connected to v1
	make_edges_relaxed(num_v, vertex_1, adjacency_matrix, cost_matrix, current_distance_array, parent_array);

	// make sure to add only the cheapest edges until shortest path is created
	while(!bool_is_array_full(num_v, min_path_found_array)){
		w = choose_min_path(num_v, min_path_found_array, adjacency_matrix, cost_matrix);
		make_edges_relaxed(num_v, w, adjacency_matrix, cost_matrix, current_distance_array, parent_array);
	}


	find_the_path_backwards(num_v, parent_array);
	*total_weight = current_distance_array[vertex_2];


	/* Change this as needed */
	return 0;
}
