#pragma once

#ifndef __LoopSubdivision_h__
#define __LoopSubdivision_h__

#include <unordered_map>	/* C++ std map */
#include <vector>			/* C++ std vector */
#include "Mesh.h"			/* mesh data structure */

#define Edge Vector2i		/* data structure for an edge: Vector2 with each component as an integer */
#define Vertex Vector3		/* data structure for a vertex: Vector3 with each component as a floating-point number */
#define Triangle Vector3i	/* data structure for a triangle: Vector3i with each component as an integer number */

// This function sort the two vertices of the input edge to make it unique to be used as a map key
inline void SortEdgeVertices(Edge &edge) 
{
	if (edge[0] > edge[1]){
		int tmp = edge[0];
		edge[0] = edge[1];
		edge[1] = tmp;
	}
}

/////////////////////////////////////////////////////
// Step 2.(1) function: find the opposite vertex in a triangle given two vertices v0 and v1
// Your task is to return the vertex index in tri that is neither v0 nor v1; return -1 if none of the vertices satisfies
/////////////////////////////////////////////////////

inline int FindTheOtherVertex(const Triangle &tri, int v0, int v1) 
{
    /* your implementation for 2.(1) starts */	


	/* your implementation for 2.(1) ends */
		
	return -1;	// return -1 if cannot find the other vertex except v0 and v1
}

/////////////////////////////////////////////////////
// Step 2.(2) function: calculate the smoothed odd vertex position using the two incident vertices p0 and p1 and two opposite vertices p2 and p3 
/////////////////////////////////////////////////////

inline Vertex SmoothOddVtx(const std::vector<Vertex> &old_vtx, const int p0, const int p1, const int p2, const int p3) 
{
	Vertex smoothed_odd_vtx = Vertex(0.0,0.0,0.0);

    /* your implementation for 2.(2) starts */	


    /* your implementation for 2.(2) ends */	
	
	return smoothed_odd_vtx;
}


/////////////////////////////////////////////////////
// Step 3.(1) function: calculate the smoothed even vertex position using the vertex position array old_vtx, the vertex index v, and the neighbor indicies nbs
/////////////////////////////////////////////////////

inline Vertex SmoothEvenVtx(const std::vector<Vertex> &old_vtx, int v, const std::vector<int> &nbs) 
{
	Vertex smoothed_even_vtx = Vertex(0.0,0.0,0.0);

    /* your implementation for 3.(1) starts */	


    /* your implementation for 3.(1) ends */	

	return smoothed_even_vtx;
}

inline void LoopSubdivision(TriangleMesh<3> &mesh) 
{
	std::vector<Vertex> &old_vtx = mesh.Vertices();				// get the reference of the mesh vertex array
	std::vector<Triangle> &old_tri = mesh.Elements();			// get the reference of the mesh triangle array
	std::vector<Vertex> new_vtx = old_vtx; 						// initialize the new_vtx array with all vertices in the old mesh
	std::vector<Triangle> new_tri;		   						// initialize the new_tri array as empty
	// Your tasks in this assignment are to update new_vtx array and the new_tri array to implement Loop's subdivision

	/////////////////////////////////////////////////////
	// Auxiliary data structures
	/////////////////////////////////////////////////////
	std::unordered_map<Edge, int> edge_vtx_map;					// auxiliary structure for edge-vertex map
	std::unordered_map<Edge, std::vector<int>> edge_tri_map;	// auxiliary structure for edge-triangle map
	std::unordered_map<int, std::vector<int>> vtx_vtx_map;		// auxiliary structure for vertex-vertex map

	/////////////////////////////////////////////////////
	// Step 1: Add new vertices and update triangles 
	// You have two tasks for this step: 
	// (1) add a new vertex to new_vtx for each edge on the old mesh; 
	// (2) add four triangles ot new_tri for each triangle on the old mesh.
	// For (1), you will add a new vertex (odd vertex) to the array of new_vtx for each edge in the old mesh. 
	// The new vertex's position is the mid-point of the edge. The new vertex's index in the index of this vertex in new_vtx array. 
	// For (2), you will add four sub-triangles to the array of new_tri for each triangle in the old mesh.
	// Be careful with the orientation of each sub-triangle to make sure the three vertices follow a counterclockwise order. 
	/////////////////////////////////////////////////////

	// loop through all triangles in the old_tri list
	for (int i = 0; i < old_tri.size(); i++){
		const Triangle &tri = old_tri[i];	// get the three vertex indices of the i-th old triangle
		Triangle tri_mid;					// create a temporal variable tri_mid to store the three new vertex indices added on each edge of the old triangle

		// loop through the three edges of the old triangle
		for (int j = 0; j < 3; j++){
			Edge edge(tri[j], tri[(j + 1) % 3]);	// get the two vertex indices of each edge
			SortEdgeVertices(edge);					// make sure the two vertex indices are sorted before using them as a map key
			
			Vertex new_vtx_pos;						// the new vertex position you want to calculate 	
			int new_vtx_idx=-1;						// the new vertex index you want to calculate (default value -1)

			// Step 1.(1): check if this edge has already been processed in previous iterations
			// if not (i.e., the edge is not in the edge_vtx_map), implement the following four steps:
			// (a) create a new vertex (odd vertex) whose position is the mid-point of the edge,
			// (b) store the position of this odd vertex into the variable new_vtx_pos,
			// (c) add new_vtx_pos into the array of new_vtx
			// (d) store the index of this odd vertex in the array of new_vtx into the variable new_vtx_idx, which will be used to update the edge_vtx_map

			if (edge_vtx_map.find(edge) == edge_vtx_map.end()){

				/* your implementation for 1.(1) starts */
				
				
				/* your implementation for 1.(1) ends */
				
				edge_vtx_map[edge] = new_vtx_idx;	// update the edge_vtx_map with the index of the added vertex
			}

			tri_mid[j] = edge_vtx_map[edge];		// update the j-th vertex in tri_mid with the index of the vertex index read from the edge_vtx_map 
		}

		// Step 1.(2): You will add four new triangles to new_tri. Make sure all triangles follow a counterclockwise order.
		
		/* your implementation for 1.(2) starts */
		

		/* your implementation for 1.(2) ends */
	}

	/////////////////////////////////////////////////////
	// Step 2: Update odd vertex position
	// You have three tasks for this step: 
	// (1) Implement the FindTheOtherVertex() function to find the opposite vertex of a given edge in a trianlge; 
	// (2) Implement the SmoothOddVtx() function to calculate the smoothed position of an odd vertex as .375*(A+B)+.125*(C+D), with A, B as the two incident vertices and C, D as the two opposite vertices
	// (3) Call SmoothOddVtx() in the for-loop to update each odd vertex's position.
	/////////////////////////////////////////////////////

	// update the edge_tri_map to store the (two) incident triangles of each edge in the auxiliary data structure 
	for (int i = 0; i < old_tri.size(); i++){
		const Triangle &tri = old_tri[i];
		for (int j = 0; j < 3; j++){
			Edge edge(tri[j], tri[(j + 1) % 3]);
			SortEdgeVertices(edge);
			edge_tri_map[edge].push_back(i);
		}
	}

	// loop through all edges on the old mesh
	for (auto &iter : edge_vtx_map){
		// read the indices of its two endpoints into p0 and p1, and the index of its cooresponding odd vertex (the newly added vertex in Step I) into v 
		Edge e = iter.first;
		int p0 = e[0];
		int p1 = e[1];
		int v = iter.second;

		// read the two incident triangles of the edge
		const std::vector<int> &edge_inc_tris = edge_tri_map[e]; // edge incident triangles
		// skip the rest of the steps if the number of the edge's incident triangles is not two (on the boundary on non-manifold) 
		if (edge_inc_tris.size() != 2){
			continue;
		}

		// Step 2.(1): implement the function FindTheOtherVertex() (see function declared above)

		// for each of the two incident triangles, calculate the index of the other vertex (except p0 and p1) and set its index value to p2 (or p3)
		int p2 = FindTheOtherVertex(old_tri[edge_inc_tris[0]], p0, p1); // find the opposite vertice p2
		int p3 = FindTheOtherVertex(old_tri[edge_inc_tris[1]], p0, p1); // find the opposite vertice p3

		// Step 2.(2): implement the function SmoothOddVtx() (see function declared above)

		// Step 2.(3): call SmoothOddVtx() to calculate the new position for each odd vertex

		/* your implementation for 2.(3) starts */

		
		/* your implementation for 2.(3) ends */
	}

	/////////////////////////////////////////////////////
	// Step 3: Update even vertex position
	// Your have two tasks for this step: 
	// (1) Implement the SmoothEvenVtx() function to calculate the smoothed position of each even vertex by taking the indices of a given vertex and its neighbors as input 
	// (2) Call the SmoothEvenVtx() function in the for-loop to update the position of each even vertex stored in new_vtx
	/////////////////////////////////////////////////////

	// update the vtx_vtx_map to store the incident vertices of each vertex in the auxiliary data structure
	// notice that here all the vertices are on the old mesh (i.e., they are all *even* vertices)
	for (auto &iter : edge_vtx_map){
		Edge e = iter.first;
		int v0 = e[0];
		int v1 = e[1];
		if (std::find(vtx_vtx_map[v0].begin(), vtx_vtx_map[v0].end(), v1) == vtx_vtx_map[v0].end()){
			vtx_vtx_map[v0].push_back(v1);
		}
		if (std::find(vtx_vtx_map[v1].begin(), vtx_vtx_map[v1].end(), v0) == vtx_vtx_map[v1].end()){
			vtx_vtx_map[v1].push_back(v0);
		}
	}

	// loop through all vertices on the old mesh
	for (auto &iter : vtx_vtx_map){
		int v = iter.first;									  	// get the vertex index
		const std::vector<int> &nbs = iter.second;			  	// get the array of its incident vertex indicies
		
		// Step 3.(1) implement the function SmoothEvenVtx() (see function declared above)
		
		// Step 3.(2) call SmoothEvenVtx() to calculate the new position for each even vertex

		/* your implementation for 3.(2) starts */
		
		/* your implementation for 3.(2) ends */
	}

	// refresh old_vtx and old_tri with the updated values in new_vtx and new_tri 
	old_vtx = new_vtx;
	old_tri = new_tri;
}


#endif