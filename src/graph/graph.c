#include <element.h>
#include <stdio.h>
#include <stdlib.h>
#include <file_reader.h>
#include <dB_graph.h>

int main(int argc, char **argv){

  FILE *fp_fnames;
  char filename[1000];
  int hash_key_bits;
  dBGraph * db_graph = NULL; 
  short kmer_size;
  int bucket_size;
  int action; //0 dump graph - 1 call SNPs
  
  FILE * fout; //binary output

   //command line arguments 
  fp_fnames= fopen(argv[1], "r");    //open file of file names
  kmer_size        = atoi(argv[2]); 
  hash_key_bits    = atoi(argv[3]);  //number of buckets: 2^hash_key_bits
  bucket_size      = atoi(argv[4]);
  action           = atoi(argv[5]);
  DEBUG            = atoi(argv[6]);
  fout             = fopen(argv[7],"w"); //output file if dump binary


  fprintf(stderr,"Input file of filenames: %s - action: %i\n",argv[1],action);
  fprintf(stderr,"Kmer size: %d hash_table_size (%d bits): %d - bucket size: %d - total size: %qd\n",kmer_size,hash_key_bits,1 << hash_key_bits, bucket_size, ((long long) 1<<hash_key_bits)*bucket_size);

  //Create the de Bruijn graph/hash table
  db_graph = hash_table_new(hash_key_bits,bucket_size, 10,kmer_size);
  fprintf(stderr,"table created: %d\n",1 << hash_key_bits);

  int count_file   = 0;
  long long total_length = 0; //total sequence length

  //Go through all the files, loading data into the graph
  while (!feof(fp_fnames)){

    fscanf(fp_fnames, "%s\n", filename);
    
    long long seq_length = 0;
    count_file++;

    seq_length += load_binary_data_from_filename_into_graph(filename,db_graph);

    total_length += seq_length;
    
    fprintf(stderr,"\n%i kmers: %qd file name:%s seq:%qd total seq:%qd\n\n",count_file,hash_table_get_unique_kmers(db_graph),filename,seq_length, total_length);

    //print mem status
    FILE* fmem=fopen("/proc/self/status", "r");
    char line[500];
    while (fgets(line,500,fmem) !=NULL){
      if (line[0] == 'V' && line[1] == 'm'){
	fprintf(stderr,"%s",line);
      }
    }
    fclose(fmem);
    fprintf(stderr,"************\n");
  }  
    
  
  int count_snps = 0;


  //routine to get SNPS --> perhaps this should go to a library
  void get_snps(dBNode * node){
    
    Orientation orientation,end_orientation;
    Nucleotide base1,base2;
    Nucleotide labels[db_graph->kmer_size];
    dBNode * end_node;

    if (db_graph_detect_perfect_bubble(node,&orientation,&base1,&base2,labels,&end_node,&end_orientation,db_graph)){
      
      int length_flank5p = 0;
      int length_flank3p = 0;
      dBNode * nodes5p[100];
      dBNode * nodes3p[100];
      Orientation orientations5p[100];
      Orientation orientations3p[100];
      Nucleotide labels_flank5p[100]; 
      Nucleotide labels_flank3p[100];
      boolean is_cycle5p, is_cycle3p;
      char tmp_seq[db_graph->kmer_size];
      int i;

      printf("SNP: %i - coverage: %d\n",count_snps,element_get_coverage(node));
      count_snps++;

      printf("five prime end\n");
      length_flank5p = db_graph_get_perfect_path(node,opposite_orientation(orientation),nodes5p,orientations5p,labels_flank5p,
						 &is_cycle5p,100,visited,db_graph);  
      printf("length 5p flank: %i\n",length_flank5p);

      printf("three prime end\n");
      length_flank3p = db_graph_get_perfect_path(end_node,end_orientation,nodes3p,orientations3p,labels_flank3p,
						 &is_cycle3p,100,visited,db_graph);    
      printf("length 3p flank: %i\n",length_flank3p);

      //print flank5p
      for(i=length_flank5p-1;i>=0;i--){
	printf("%c",reverse_char_nucleotide(binary_nucleotide_to_char(labels_flank5p[i])));
      }

      //print the initial node
      printf(" ");
      if (orientation == forward){
	printf ("%s ",binary_kmer_to_seq(element_get_kmer(node),db_graph->kmer_size,tmp_seq));
      }
      else{
	printf ("%s ",binary_kmer_to_seq(binary_kmer_reverse_complement(element_get_kmer(node),db_graph->kmer_size),db_graph->kmer_size,tmp_seq));
      }

      printf (" [%c %c] ",binary_nucleotide_to_char(base1),binary_nucleotide_to_char(base2));
      
      //print bubble
      for(i=0;i<db_graph->kmer_size;i++){
	printf("%c",binary_nucleotide_to_char(labels[i]));
      }

      printf(" ");
      //print flank3p
      for(i=0;i<length_flank3p;i++){
	printf("%c",binary_nucleotide_to_char(labels_flank3p[i]));
      }

      printf("\n");
    }
  }    


  //routine to dump graph
  void print_node_binary(dBNode * node){
    db_node_print_binary(fout,node);
  }

  switch (action){
  case 0 :
    printf("dumping graph %s\n",argv[7]);
    hash_table_traverse(&print_node_binary,db_graph); 
    break;

  case 1 :
    printf("call SNPs\n");
    hash_table_traverse(&get_snps,db_graph);
    break;
  }

  return 0;
}
