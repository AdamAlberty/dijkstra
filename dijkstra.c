#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct Vertex;

// Holds information about distance to adjacent vertex
typedef struct AdjacentVertex {
    struct Vertex *vertex;
    int distance;
} AdjacentVertex;

// Holds information about graph vertex
typedef struct Vertex {
    char *label;
    int adjacent_count;
    AdjacentVertex **adjacent_vertices;
} Vertex;

// Holds information about graph
typedef struct Graph {
    int num_of_vertices;
    Vertex **vertices;
} Graph;

// Holds information about labels for shortest path
typedef struct Marking {
    char *label;
    int distance;
    int is_explored;
    struct Marking *from;
} Marking;

// Find vertex based on label
Vertex *find_vertex(Graph *graph, char *label) {
    for (int i = 0; i < graph->num_of_vertices; i++) {
        if (strcmp(graph->vertices[i]->label, label) == 0) {
            return graph->vertices[i];
        }
    }
    return NULL;
}

// Creates new vertex
Vertex* create_vertex(char *vertex_label) { 
    char *label = malloc(strlen(vertex_label) + 1 * sizeof(char));
    strcpy(label, vertex_label);
    Vertex *new_vertex = malloc(sizeof(Vertex));
    new_vertex->label = label;
    new_vertex->adjacent_vertices = NULL;
    new_vertex->adjacent_count = 0;
    return new_vertex;
}

// Creates edge between vertices based on label
void create_edge(Graph *graph, Vertex *node, char *label, int distance) {
    Vertex *found_vertex = find_vertex(graph, label);
    if (found_vertex == NULL) {
        perror("Could not create edge, node with that label does not exist");
        exit(1);
    }

    if (node->adjacent_count == 0) {
        node->adjacent_vertices = malloc(sizeof(AdjacentVertex*));
        if (node->adjacent_vertices == NULL) {
            perror("Could not allocate");
            exit(1);
        }

        node->adjacent_vertices[0] = malloc(sizeof(AdjacentVertex));
        if (node->adjacent_vertices[0] == NULL) {
            perror("Could not create edge link");
            exit(1);
        }

        node->adjacent_vertices[0]->distance = distance;
        node->adjacent_vertices[0]->vertex = found_vertex;
        node->adjacent_count++;
        return;
    }

    node->adjacent_count += 1;
    node->adjacent_vertices = realloc(node->adjacent_vertices, sizeof(AdjacentVertex*) * (node->adjacent_count));
    if (node->adjacent_vertices == NULL) {
        perror("Could not reallocate vertices");
        exit(1);
    }

    node->adjacent_vertices[node->adjacent_count - 1] = malloc(sizeof(AdjacentVertex));
    if (node->adjacent_vertices[node->adjacent_count - 1] == NULL) {
        perror("Error adding new edge");
        exit(1);
    }
    node->adjacent_vertices[node->adjacent_count - 1]->distance = distance;
    node->adjacent_vertices[node->adjacent_count - 1]->vertex = found_vertex;
}

// Creates graph representation from file
void create_graph(Graph *graph) {
    graph->vertices = NULL;
    graph->num_of_vertices = 0;
    
    FILE *fp = fopen("./nodes.txt", "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(1);
    }

    char buffer[50];

    // Create vertices
    while (fgets(buffer, 50, fp) != NULL) {
        if (buffer[0] == '#') continue;
        graph->vertices = malloc(sizeof(Vertex*));

        int i = 0;
        while(buffer[i] != '\n') {
            if (buffer[i] == ' ') {
                i++;
                continue;
            }

            char label[50];
            label[0] = buffer[i];
            label[1] = '\0';

            graph->num_of_vertices+= 1;
            graph->vertices = realloc(graph->vertices, sizeof(Vertex *) * graph->num_of_vertices);
            Vertex *new_vertex = create_vertex(label);
            graph->vertices[graph->num_of_vertices - 1] = new_vertex;
            i++;
        }
        break;
    }

    // setup relationships between vertices
    for (int vert_num = 0; vert_num < graph->num_of_vertices; vert_num++) {
        if(fgets(buffer, 50, fp) == NULL) {
            perror("Too few lines for the number of vertices");
            exit(1);
        }
        if (buffer[0] == '#') {
            vert_num--;
            continue;
        }

        int i = 0;
        while (i < strlen(buffer) - 1) {
            int dist;
            char label[50];
            sscanf(buffer + i, "%d %s", &dist, label);
            create_edge(graph, graph->vertices[vert_num], label, dist);
            i += 4;
    }}


    fclose(fp);
}

// Print loaded graph
void print_graph(Graph *graph) {
    printf("\n-----------------GRAPH-----------------\n");
    for (int i = 0; i < graph->num_of_vertices; i++) {
        printf("%s: ", graph->vertices[i]->label);
        for (int j = 0; j < graph->vertices[i]->adjacent_count; j++) {
            printf("%s(%d), ", graph->vertices[i]->adjacent_vertices[j]->vertex->label, graph->vertices[i]->adjacent_vertices[j]->distance);
        }
        printf("\n");
    }
    printf("---------------------------------------\n\n");
}


// Print shortest paths in the terminal
void print_shortest_paths(char *label_starting, Graph *graph, Marking **markings) {
    printf("\n\n-----------SHORTEST PATHS--------------\n");
    printf("Starting vertex: %s\n", label_starting);
    for (int i = 0; i < graph->num_of_vertices; i++) {
        printf("Label: %s\n", markings[i]->label);
        printf("Distance: %d\n", markings[i]->distance);
        printf("From: %s\n", markings[i]->from == NULL ? "-" : markings[i]->from->label);
        printf("Is explored: %d\n", markings[i]->is_explored);
        printf("\n");
    }
    printf("----------------------------------------");
}


Marking *get_marking_by_label(Graph *graph, Marking **markings, char *label) {
    for (int i = 0; i < graph->num_of_vertices; i++) {
        if (strcmp(markings[i]->label, label) == 0) {
            return markings[i];
        }
    }
    perror("Could not find marking by label");
    exit(1);
}

int get_vertex_idx_by_label(Graph *graph, char *label) {
    for (int i = 0; i < graph->num_of_vertices; i++) {
        if (strcmp(graph->vertices[i]->label, label) == 0) {
            return i;
        }
    }
    perror("Could not find vertex id by label");
    exit(1);
}

int get_shortest_unexplored_vertex_idx(Graph *graph, Marking **markings) {
    int shortest_idx = -1;
    for (int i = 0; i < graph->num_of_vertices; i++) {
        if (markings[i]->is_explored || markings[i]->distance == -1) {
            continue;
        }
        if (shortest_idx == -1) {
            shortest_idx = i;
        } else if (markings[i]->distance < markings[shortest_idx]->distance) {
            shortest_idx = i;
        }
    }
    return shortest_idx;
}

int get_next_vertex(Graph *graph, Marking** markings, Vertex *current_vertex, Marking *current_marking) {
        // Loop through neighbors of currently explored vertex
        for (int i = 0; i < current_vertex->adjacent_count; i++) {
            Marking *marking = get_marking_by_label(graph, markings, current_vertex->adjacent_vertices[i]->vertex->label);
            // Skip already explored vertices
            if (marking->is_explored) {
                continue;
            }

            int distance = current_marking->distance + current_vertex->adjacent_vertices[i]->distance;

            // Change label if it doesn't have value
            if (marking->distance == -1) {
                marking->distance = distance;
                marking->from = current_marking;
            } else if (distance < marking->distance) {
                marking->distance = distance;
                marking->from = current_marking;
            }
        }

        return get_shortest_unexplored_vertex_idx(graph, markings);
}

void free_markings(Graph *graph, Marking **markings) {
    for (int i = 0; i < graph->num_of_vertices; i++) {
        free(markings[i]);
    }
    free(markings);
}

void shortest_path(Graph *graph, char *label_starting) {
    Vertex *current_vertex = find_vertex(graph, label_starting);
    Marking **markings = malloc(sizeof(Marking*) * graph->num_of_vertices);

    int current_vertex_idx = 0;

    // Initialize markings
    for (int i = 0; i < graph->num_of_vertices; i++) {
        markings[i] = malloc(sizeof(Marking));
        markings[i]->label = graph->vertices[i]->label;
        markings[i]->from = NULL;

        if (strcmp(current_vertex->label, markings[i]->label) == 0) {
            markings[i]->distance = 0;
            markings[i]->is_explored = 1;
            current_vertex_idx = i;
        } else {
            markings[i]->distance = -1; // Signal infinity
            markings[i]->is_explored = 0;
        }
    }

    while (current_vertex_idx != -1) {
        Vertex *current_vertex = graph->vertices[current_vertex_idx];
        markings[current_vertex_idx]->is_explored = 1;
        current_vertex_idx = get_next_vertex(graph, markings, current_vertex, markings[current_vertex_idx]);
    }

    print_shortest_paths(label_starting, graph, markings);
    free_markings(graph, markings);
}

void free_graph(Graph *graph) {
    for (int i = 0; i < graph->num_of_vertices; i++) {
        Vertex *currentVertex = graph->vertices[i];
        for (int j = 0; j < currentVertex->adjacent_count; j++) {
            free(currentVertex->adjacent_vertices[j]);
        }
        free(currentVertex->adjacent_vertices);
        free(currentVertex->label);
        free(currentVertex);
    }
    free(graph->vertices);
}


int main(void) {
    Graph graph;
    create_graph(&graph);
    print_graph(&graph);
    shortest_path(&graph, "A");
    free_graph(&graph);
}