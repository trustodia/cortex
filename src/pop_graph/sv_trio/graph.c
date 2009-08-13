#include <element.h>
#include <stdio.h>
#include <stdlib.h>
#include <file_reader.h>
#include <dB_graph.h>
#include <dB_graph_population.h>

int main(int argc, char **argv){

  char* filename;
  int hash_key_bits, bucket_size;
  dBGraph * db_graph = NULL; 
  short kmer_size;
  int action;
  char* specific_fasta;

  //command line arguments 
  filename         = argv[1];        //open file that lists one file per individual in the trio (population), and each of those gives list of files.
  kmer_size        = atoi(argv[2]);  //global variable defined in element.h
  hash_key_bits    = atoi(argv[3]);  //number of buckets: 2^hash_key_bits
  bucket_size      = atoi(argv[4]);
  action           = atoi(argv[5]);
  DEBUG            = atoi(argv[6]);
  specific_fasta   = argv[7];

  int max_retries=10;

  fprintf(stdout,"Kmer size: %d hash_table_size (%d bits): %d\n",kmer_size,hash_key_bits,1 << hash_key_bits);


  //Create the de Bruijn graph/hash table
  db_graph = hash_table_new(hash_key_bits,bucket_size, max_retries, kmer_size);
  fprintf(stderr,"table created: %d\n",1 << hash_key_bits);

 
  fprintf(stderr, "Start loading population\n");
  load_population_as_binaries_from_graph(filename, db_graph);
  fprintf(stderr, "Finished loading population\n");

  // ***************************************
  //locations of chromosome reference files:
  // ***************************************

  char** ref_chroms = malloc( sizeof(char*) * 25); //one for each chromosome, ignoring haplotypes like MHC
  if (ref_chroms==NULL)
    {
      printf("OOM. Give up can't even allocate space for the names of the ref chromosome files\n");
      exit(1);
    }
  int i;
  for (i=0; i< 25; i++)
    {
      ref_chroms[i] = malloc(sizeof(char)*150); //filenames including path are about 100 characters. 50 characters of leeway
      if (ref_chroms[i]==NULL)
	{
	  printf("OOM. Giveup can't even allocate space for the names of the ref chromosome file i = %d\n",i);
	  exit(1);
	}
    }

  ref_chroms[0] = "/nfs/1000g-work/G1K/work/zi/projects/marzam/humref/Homo_sapiens.NCBI36.52.dna.chromosome.MT.fa";
  
  for (i=1; i<23; i++)
    {
      sprintf(ref_chroms[i], "/nfs/1000g-work/G1K/work/zi/projects/marzam/humref/Homo_sapiens.NCBI36.52.dna.chromosome.%i.fa", i);
    }
  ref_chroms[23]="/nfs/1000g-work/G1K/work/zi/projects/marzam/humref/Homo_sapiens.NCBI36.52.dna.chromosome.X.fa";
  ref_chroms[24]="/nfs/1000g-work/G1K/work/zi/projects/marzam/humref/Homo_sapiens.NCBI36.52.dna.chromosome.Y.fa";



  // **** set up one output file per chromosome ***** //
  
  char** output_files = malloc( sizeof(char*) * 25); //one for each chromosome, ignoring haplotypes like MHC
  if (output_files==NULL)
    {
      printf("OOM. Give up can't even allocate space for the names of the output  files \n");
      exit(1);
    }

  for (i=0; i< 25; i++)
    {
      output_files[i] = malloc(sizeof(char)*100); 
      if (output_files[i]==NULL)
	{
	  printf("OOM. Giveup can't even allocate space for the names of the ref chromosome file i = %d\n",i);
	  exit(1);
	}
    }

  output_files[0] = "sv_called_in_MT";
  
  for (i=1; i<23; i++)
    {
      sprintf(output_files[i], "sv_called_in_chrom_%i", i);
    }
  output_files[23]="sv_called_in_chrom_X";
  output_files[24]="sv_called_in_chrom_Y";





  switch (action)
    {
    case 0:
      {
	printf("Make SV calls based on the trusted-path/supernode algorithm, against the whole genome\n");
 
	int min_fiveprime_flank_anchor = 2;
	int min_threeprime_flank_anchor= 21;
	int max_anchor_span =  20000;
	int length_of_arrays = 40000;
	int min_covg =1;
	int max_covg = 10000000;
	int max_expected_size_of_supernode=20000;
      
      
      //ignore mitochondrion for now, so start with i=1
      for (i=1; i<25; i++) 
	{
	  printf("Call SV comparing individual with chromosome %s\n", ref_chroms[i]);
	  
	  FILE* chrom_fptr = fopen(ref_chroms[i], "r");
	  if (chrom_fptr==NULL)
	    {
	      printf("Cannot open %s \n", ref_chroms[i]);
	      exit(1);
	    }
      
	  FILE* out_fptr = fopen(output_files[i], "w");
	  if (out_fptr==NULL)
	    {
	      printf("Cannot open %s for output\n", output_files[i]);
	      exit(1);
	    }
	  
	  //Note we assume person 0 is the reference, and person 1 is the person we are interested in
	  int ret = db_graph_make_reference_path_based_sv_calls(chrom_fptr, individual_edge_array, 1, 
								individual_edge_array, 0,
								min_fiveprime_flank_anchor, min_threeprime_flank_anchor, max_anchor_span, min_covg, max_covg, 
								max_expected_size_of_supernode, length_of_arrays, db_graph, out_fptr,
								0, NULL, NULL, NULL, NULL, NULL);

	  
	  fclose(chrom_fptr);
	  fclose(out_fptr);
	}
      break;

      }

      

    }


  //cleanup

  /*
  for(i=0; i<25; i++)
    {
      free(ref_chroms[i]);
      free(output_files[i]);
    }

  free(ref_chroms);
  free(output_files);
  */

  printf("Finished getting SV from all chromosomes\n");
  return 0;
}