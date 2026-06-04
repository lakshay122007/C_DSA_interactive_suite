#include "graph_traversals.h"
#include "safe_input.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// A structure to represent a subset for union-find
typedef struct DSUSubset
{
    int parent;
    int rank;
} DSUSubset;

// Find set of an element i (uses path compression technique)
static int dsu_find(DSUSubset subsets[], int i)
{
    if (subsets[i].parent != i)
    {
        subsets[i].parent = dsu_find(subsets, subsets[i].parent);
    }
    return subsets[i].parent;
}

// A function that does union of two sets of x and y (uses union by rank)
static void dsu_union(DSUSubset subsets[], int x, int y)
{
    int xroot = dsu_find(subsets, x);
    int yroot = dsu_find(subsets, y);

    if (subsets[xroot].rank < subsets[yroot].rank)
    {
        subsets[xroot].parent = yroot;
    }
    else if (subsets[xroot].rank > subsets[yroot].rank)
    {
        subsets[yroot].parent = xroot;
    }
    else
    {
        subsets[yroot].parent = xroot;
        subsets[xroot].rank++;
    }
}

// Compare function used by qsort to sort edges in ascending order of weight
static int compare_edges(const void* a, const void* b)
{
    KruskalEdge* a_edge = (KruskalEdge*)a;
    KruskalEdge* b_edge = (KruskalEdge*)b;
    return a_edge->weight - b_edge->weight;
}

/**
 * Runs Kruskal's MST algorithm.
 * 
 * @param edges Array of all graph edges
 * @param V Number of vertices
 * @param E Number of edges
 * @param mst_edges Pre-allocated array to store selected MST edges
 * @param mst_edge_count Output pointer to store actual number of MST edges included
 */
void kruskal_mst(KruskalEdge* edges, int V, int E, KruskalEdge* mst_edges, int* mst_edge_count)
{
    *mst_edge_count = 0;

    // 1. Sort all the edges in non-decreasing order of their weight
    qsort(edges, E, sizeof(KruskalEdge), compare_edges);

    // Allocate memory for creating V subsets
    DSUSubset* subsets = malloc(V * sizeof(DSUSubset));
    if (!subsets)
    {
        printf("\nMemory allocation failed in DSU initialization.\n");
        return;
    }

    // Create V subsets with single elements
    for (int v = 0; v < V; v++)
    {
        subsets[v].parent = v;
        subsets[v].rank = 0;
    }

    int e_index = 0; // Index used to pick next edge
    int m_index = 0; // Index used to write to mst_edges

    // Number of edges to be taken is equal to V-1
    while (m_index < V - 1 && e_index < E)
    {
        // Step 2: Pick the smallest edge. And increment the index for next iteration
        KruskalEdge next_edge = edges[e_index++];

        int x = dsu_find(subsets, next_edge.src);
        int y = dsu_find(subsets, next_edge.dest);

        // If including this edge doesn't cause cycle, include it in result
        // and increment the index of result for next edge
        if (x != y)
        {
            mst_edges[m_index++] = next_edge;
            dsu_union(subsets, x, y);
        }
        // Else discard the next_edge
    }

    *mst_edge_count = m_index;
    free(subsets);
}

/**
 * Interactive demo for testing Kruskal's MST algorithm.
 */
void kruskal_demo(void)
{
    int vertices = 0;
    int edges_num = 0;

    while (1)
    {
        int status = safe_input_int(&vertices,
                                    "\nEnter the number of vertices in the graph (between 1 and 10), or enter '-1' to exit: ",
                                    1, 10);
        if (status == INPUT_EXIT_SIGNAL)
        {
            printf("\nExiting Kruskal's demo...\n");
            return;
        }
        if (status == 0)
        {
            continue;
        }
        break;
    }

    while (1)
    {
        int max_edges = vertices * (vertices - 1) / 2; // Simple undirected graph maximum edges
        if (max_edges < 0) max_edges = 0;
        
        int status = safe_input_int(&edges_num,
                                    "\nEnter the number of edges, or enter '-1' to exit: ",
                                    0, max_edges);
        if (status == INPUT_EXIT_SIGNAL)
        {
            printf("\nExiting Kruskal's demo...\n");
            return;
        }
        if (status == 0)
        {
            continue;
        }
        break;
    }

    if (edges_num == 0)
    {
        printf("\nGraph has 0 edges. No spanning tree can be formed.\n");
        return;
    }

    KruskalEdge* edges = malloc(edges_num * sizeof(KruskalEdge));
    KruskalEdge* mst_edges = malloc((vertices - 1) * sizeof(KruskalEdge));

    if (!edges || !mst_edges)
    {
        printf("\nMemory allocation failed.\n");
        if (edges) free(edges);
        if (mst_edges) free(mst_edges);
        return;
    }

    printf("\nEnter source, destination, and weight for each undirected edge:\n");
    printf("(Vertices must be between 0 and %d. Weights can be negative)\n", vertices - 1);

    for (int i = 0; i < edges_num; i++)
    {
        int src = 0, dest = 0, weight = 0;
        int status;

    retry_src:
        status = safe_input_int(&src, "Source node: ", 0, vertices - 1);
        if (status == INPUT_EXIT_SIGNAL)
        {
            goto cleanup;
        }
        if (status == 0)
        {
            goto retry_src;
        }

    retry_dest:
        status = safe_input_int(&dest, "Destination node: ", 0, vertices - 1);
        if (status == INPUT_EXIT_SIGNAL)
        {
            goto cleanup;
        }
        if (status == 0)
        {
            goto retry_dest;
        }

    retry_weight:
        status = safe_input_int(&weight, "Edge weight (e.g. -1000 to 1000): ", -1000, 1000);
        if (status == INPUT_EXIT_SIGNAL)
        {
            goto cleanup;
        }
        if (status == 0)
        {
            goto retry_weight;
        }

        edges[i].src = src;
        edges[i].dest = dest;
        edges[i].weight = weight;
    }

    int mst_edge_count = 0;
    kruskal_mst(edges, vertices, edges_num, mst_edges, &mst_edge_count);

    printf("\nFollowing are the edges in the constructed MST:\n");
    printf("Source  <->  Destination   \t   Weight\n");
    printf("------       -----------   \t   ------\n");

    int total_weight = 0;
    for (int i = 0; i < mst_edge_count; i++)
    {
        printf("  %d    <->       %d       \t     %d\n", mst_edges[i].src, mst_edges[i].dest, mst_edges[i].weight);
        total_weight += mst_edges[i].weight;
    }

    printf("\nTotal weight of Minimum Spanning Tree: %d\n", total_weight);

    if (mst_edge_count < vertices - 1)
    {
        printf("Warning: The graph is disconnected! The output forms a Spanning Forest containing %d connected components.\n",
               vertices - mst_edge_count);
    }

cleanup:
    free(edges);
    free(mst_edges);
}
