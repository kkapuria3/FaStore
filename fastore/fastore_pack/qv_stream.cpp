/*
  This file is a part of FaStore software distributed under GNU GPL 2 licence.
  The code in this file is based on QVZ software.

  Github:	https://github.com/refresh-bio/FaStore

  Authors: Lukasz Roguski, Idoia Ochoa, Mikel Hernaez & Sebastian Deorowicz
*/

#include "qv_compressor.h"


/**
 * Update stats structure used for adaptive arithmetic coding
 * @param stats Pointer to stats structure
 * @param x Symbol to update
 * @param r Rescaling condition (if n > r, rescale all stats)
 */
void update_stats(stream_stats_ptr_t stats, uint32_t x, uint32_t r) {
    uint32_t i = 0;

	stats->counts[x] += stats->step;
	stats->n += stats->step;

	if (stats->n > r) {
		stats->n = 0;
		for (i = 0; i < stats->alphabetCard; ++i) {
			if (stats->counts[i]) {
				stats->counts[i] >>= 1;
				stats->counts[i] += 1;
				stats->n += stats->counts[i];
			}
		}
	}
}

/**
 * Initialize stats structures used for adaptive arithmetic coding based on
 * the number of contexts required to handle the set of conditional quantizers
 * that we have (one context per quantizer)
 */
stream_stats_ptr_t **initialize_stream_stats(struct cond_quantizer_list_t *q_list) {
    stream_stats_ptr_t **s;
    uint32_t i = 0, j = 0, k = 0;
    
    s = (stream_stats_ptr_t **) calloc(q_list->columns, sizeof(stream_stats_ptr_t *));

    // Allocate jagged array, one set of stats per column
    for (i = 0; i < q_list->columns; ++i) {
		// And for each column, one set of stats per low/high quantizer per previous context
        s[i] = (stream_stats_ptr_t *) calloc(2*q_list->input_alphabets[i]->size, sizeof(stream_stats_ptr_t));
        
		// Finally each individual stat structure needs to be filled in uniformly
        for (j = 0; j < 2*q_list->input_alphabets[i]->size; ++j) {
            s[i][j] = (stream_stats_ptr_t) calloc(1, sizeof(struct stream_stats_t));
            s[i][j]->counts = (uint32_t *) calloc(q_list->q[i][j]->output_alphabet->size, sizeof(uint32_t));
            
            // Initialize the quantizer's stats uniformly
            for (k = 0; k < q_list->q[i][j]->output_alphabet->size; k++) {
                s[i][j]->counts[k] = 1;
            }
			s[i][j]->n = q_list->q[i][j]->output_alphabet->size;
            s[i][j]->alphabetCard = q_list->q[i][j]->output_alphabet->size;
            
            // Step size is 8 counts per symbol seen to speed convergence
            s[i][j]->step = 8;
        }
    }
    
    return s;
}

/**
 * @todo add cluster stats
 */
bool initialize_qvz_compressor(arithStream as, struct cond_quantizer_list_t *qlist)
{
	ASSERT(as != NULL);
	//qv_c = calloc(1, sizeof(struct qv_compressor_t));
	//uint32_t i;

	as->cluster_stats = (stream_stats_ptr_t) calloc(1, sizeof(struct stream_stats_t));
	as->cluster_stats->step = 8;
	as->cluster_stats->counts = (uint32_t *) calloc(1, sizeof(uint32_t));
	as->cluster_stats->alphabetCard = 1;
	as->cluster_stats->n = 1;

	as->stats = (stream_stats_ptr_t ***) calloc(1, sizeof(stream_stats_ptr_t **));

	as->stats[0] = initialize_stream_stats(qlist);
	as->cluster_stats->counts[0] = 1;
    
	//as->a = initialize_arithmetic_encoder(m_arith);
	//as->a->t = 0;
    
	return true;
}

void free_qvz_arith(arithStream as, struct cond_quantizer_list_t *qlist)
{
	free(as->cluster_stats->counts);
	free(as->cluster_stats);

	stream_stats_ptr_t **s = as->stats[0];
	for (uint32 i = 0; i < qlist->columns; ++i)
	{
		for (uint32 j = 0; j < 2*qlist->input_alphabets[i]->size; ++j)
		{
			free(s[i][j]->counts);
			free(s[i][j]);
		}

		free(s[i]);
	}
	free(as->stats[0]);
	free(as->stats);
}


bool initialize_qvz_decompressor(arithStream as, struct cond_quantizer_list_t *qlist)
{
	ASSERT(as != NULL);
	//qv_c = calloc(1, sizeof(struct qv_compressor_t));

	//arithStream as;
	//uint32_t i;
	//as = (arithStream) calloc(1, sizeof(struct arithStream_t));

	as->cluster_stats = (stream_stats_ptr_t) calloc(1, sizeof(struct stream_stats_t));
	as->cluster_stats->step = 8;
	as->cluster_stats->counts = (uint32_t *) calloc(1, sizeof(uint32_t));
	as->cluster_stats->alphabetCard = 1;
	as->cluster_stats->n = 1;

	as->stats = (stream_stats_ptr_t ***) calloc(1, sizeof(stream_stats_ptr_t **));
	as->stats[0] = initialize_stream_stats(qlist);
	as->cluster_stats->counts[0] = 1;


	//as->a = initialize_arithmetic_encoder(m_arith);


	//as->a->t = stream_read_bits(as->os, as->a->m);
	//as->is = reader;
	//as->a->t = as->is->GetBits(as->a->m);

	return true;
}

/*
qv_compressor initialize_qv_compressor(FILE *fout, uint8_t streamDirection, struct quality_file_t *info) {
    qv_compressor s;
    s = calloc(1, sizeof(struct qv_compressor_t));
    printf("check 1\n");
    s->Quals = initialize_arithStream(fout, streamDirection, info);
    return s;
}
*/
