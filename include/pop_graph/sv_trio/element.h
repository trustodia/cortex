/*
  element.h defines the interface for the de Bruijn graph node. The implementation is complemented by 
  a hash table that stores every node indexed by kmers (BinaryKmers). 

  The element routines, ie the one required by hash_table/priority queue, are prefixed with element_ 
  The de Bruijn based routines are prefixed with db_node
*/

#ifndef ELEMENT_H_
#define ELEMENT_H_


#include <binary_kmer.h>
#include <global.h>
#include <pop_globals.h>
#include <stdio.h>

//type definitions

typedef char Edges;

typedef enum
  {
    unassigned   = 0,
    none         = 1,
    visited      = 2,
    pruned_from_NA12878  = 3,
    pruned_from_NA12891  = 4,
    pruned_from_NA12892  = 5,
    pruned_from_NA12878_and_NA12891  = 6,
    pruned_from_NA12878_and_NA12892  = 7,
    pruned_from_NA12891_and_NA12892  = 8,
    pruned_from_NA12878_and_NA12891_and_NA12892  = 9,
    exists_in_reference = 10,
    visited_and_exists_in_reference = 11,

  } NodeStatus;



typedef enum{
    individual_edge_array = 0,  
    ref_edge_array        = 1,
} EdgeArrayType;


typedef struct{
  BinaryKmer kmer;
  Edges individual_edges[NUMBER_OF_INDIVIDUALS_PER_POPULATION];
  short coverage[NUMBER_OF_INDIVIDUALS_PER_POPULATION];
  NodeStatus status;
} Element;


typedef Element dBNode;
typedef BinaryKmer Key;

typedef enum{
  forward = 0,
  reverse = 1
} Orientation;

typedef enum{
  overlaps_forwards_only=0,
    overlaps_reverse_only = 1,
    overlaps_both_directions =2,
    does_not_overlap =4,
    } Overlap;

typedef Element GraphNode;

Element* new_element();
void free_element(Element** element);
void element_assign(Element* e1, Element* e2);


//utility function for getting the desired edge char, by specifying if talking about a population or an individual
// and giving the appropriate index in the relevant array

Edges* get_edge(Element, EdgeArrayType, int); //gets pointer to actual edge, so you can modify it
Edges get_edge_copy(const Element e, EdgeArrayType type,int index); //gets copy of edge
Edges get_union_of_edges(Element e);
void add_edges(Element*, EdgeArrayType, int, Edges);
void set_edges(Element*, EdgeArrayType, int, Edges);
void reset_one_edge(Element* e, Orientation orientation, Nucleotide nucleotide, EdgeArrayType type, int index);

int element_get_number_of_people_or_pops_containing_this_element(Element* e, EdgeArrayType type, int index);


boolean element_smaller(Element,Element);
BinaryKmer element_get_kmer(Element *);
boolean element_is_key(Key,Element, short kmer_size);
Key element_get_key(BinaryKmer,short kmer_size);
void element_initialise(Element *,Key, short kmer_size);

//reverse orientation
Orientation opposite_orientation(Orientation);
Orientation db_node_get_orientation(BinaryKmer, dBNode *, short kmer_size);

//add an edge between nodes -- NB: it adds both edges: forward and reverse
boolean db_node_add_edge(dBNode *, dBNode *, Orientation, Orientation, short kmer_size, EdgeArrayType edge_type, int edge_index); 


//returns yes if the label defined by the nucleotide coresponds to an 
//outgoing edge in the side defined by the orientation.   
boolean db_node_edge_exist(dBNode *, Nucleotide, Orientation, EdgeArrayType edge_type, int edge_index);

//returns the label of the first outgoing edge -- leaving from the side 
//defined by orientation. 
boolean db_node_has_precisely_one_edge(dBNode *, Orientation, Nucleotide *, EdgeArrayType edge_type, int edge_index);

boolean db_node_has_precisely_one_edge_in_union_graph_over_all_people(dBNode * node, Orientation orientation, Nucleotide * nucleotide);

boolean db_node_has_precisely_two_edges(dBNode * node, Orientation orientation, Nucleotide * nucleotide1, Nucleotide * nucleotide2, EdgeArrayType type, int index);

void db_node_reset_all_edges_for_all_people_and_pops_to_zero(Element* e);

//forgets about the edges
void db_node_reset_edges(dBNode *, EdgeArrayType, int  );

void db_node_reset_edge(dBNode *, Orientation, Nucleotide, EdgeArrayType, int );

//check that the edges are 0's
boolean db_node_edges_reset(dBNode * node, EdgeArrayType edge_type, int edge_index);

boolean db_node_check_status(dBNode * node, NodeStatus status);
boolean db_node_check_status_not_pruned(dBNode * node);
boolean db_node_check_status_not_pruned_or_visited(dBNode * node);
boolean db_node_check_status_is_not_visited(dBNode* node);


void db_node_set_status(dBNode * node,NodeStatus status);
void db_node_trio_aware_set_pruned_status(dBNode * node, int index);
void db_node_set_status_to_none(dBNode * node);



//actions and conditions 

void db_node_action_set_status_none(dBNode * node);

//void db_node_action_set_status_pruned(dBNode * node);
void db_node_action_set_status_visited(dBNode * node);

void db_node_action_set_status_visited_or_visited_and_exists_in_reference(dBNode * node);

void db_node_action_unset_status_visited_or_visited_and_exists_in_reference(dBNode * node);

void db_node_action_do_nothing(dBNode * node);

boolean db_node_check_status_none(dBNode * node);

boolean db_node_check_status_visited(dBNode * node);

boolean db_node_check_status_exists_in_reference(dBNode * node);

boolean db_node_check_status_visited_and_exists_in_reference(dBNode * node);

boolean db_node_check_status_is_not_exists_in_reference(dBNode * node);

boolean db_node_check_status_is_not_visited_or_visited_and_exists_in_reference(dBNode * node);

boolean db_node_condition_always_true(dBNode* node);





void db_node_increment_coverage(dBNode* e, EdgeArrayType type, int index);
void db_node_update_coverage(dBNode* e, EdgeArrayType type, int index, short update);
int db_node_get_coverage(const dBNode* const e, EdgeArrayType type, int index);
short db_node_get_coverage_as_short(dBNode* e, EdgeArrayType type, int index);



//check if node doesn't have any edges in a given orientation
boolean db_node_is_blunt_end(dBNode * node, Orientation orientation, EdgeArrayType edge_type, int edge_index);


boolean db_node_is_this_node_in_this_person_or_populations_graph(dBNode* node, EdgeArrayType type, int index);


//functions for binary format
void db_node_print_binary(FILE * fp, dBNode * node);

//read a binary dumped by this module, sv_trio
boolean db_node_read_sv_trio_binary(FILE * fp, short kmer_size, dBNode * node);

//read a binary for an individual person, as dumped by the target "graph"
// the edge array type and index tell you which person you should load this data into
boolean db_node_read_graph_binary(FILE * fp, short kmer_size, dBNode * node, EdgeArrayType type, int index);

#endif /* ELEMENT_H_ */